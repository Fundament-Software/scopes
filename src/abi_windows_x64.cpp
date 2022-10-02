/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "platform_abi.hpp"
#include "types.hpp"
#include "type.hpp"
#include "utils.hpp"
#include "dyn_cast.inc"

#include <assert.h>

#pragma GCC diagnostic ignored "-Wvla-extension"

// The windows ABI is identical to the standard x86-64 ABI, except that structs are never
// passed into registers, regardless of size
namespace scopes {
  namespace abi_windows_x64 {
    static size_t classify(const Type* T, ABIClass* classes, size_t offset);

    static size_t classify_tuple_like(size_t size,
      const Type** fields, size_t count, bool packed,
      ABIClass* classes, size_t offset) {
        const size_t UNITS_PER_WORD = 8;
        size_t words = (size + UNITS_PER_WORD - 1) / UNITS_PER_WORD;
        if (size > 32)
          return 0;
        for (size_t i = 0; i < MAX_ABI_CLASSES; i++)
          classes[i] = ABI_CLASS_NO_CLASS;
        if (!words) {
          classes[0] = ABI_CLASS_NO_CLASS;
          return 1;
        }

        switch (size)
        {
        case 1:
          classes[0] = ABI_CLASS_INTEGERSI8;
          return 1;
        case 2:
          classes[0] = ABI_CLASS_INTEGERSI16;
          return 1;
        case 4:
          classes[0] = ABI_CLASS_INTEGERSI;
          return 1;
        case 8:
          classes[0] = ABI_CLASS_INTEGER;
          return 1;
        }
        return 0;
    }

    static size_t classify_array_like(size_t size,
      const Type* element_type, size_t count,
      ABIClass* classes, size_t offset) {
      const size_t UNITS_PER_WORD = 8;
      size_t words = (size + UNITS_PER_WORD - 1) / UNITS_PER_WORD;
      if (size > 32)
        return 0;
      for (size_t i = 0; i < MAX_ABI_CLASSES; i++)
        classes[i] = ABI_CLASS_NO_CLASS;
      if (!words) {
        classes[0] = ABI_CLASS_NO_CLASS;
        return 1;
      }
      auto ET = qualified_storage_type(element_type).assert_ok();
      ABIClass subclasses[MAX_ABI_CLASSES];
      size_t alignment = align_of(ET).assert_ok();
      size_t esize = size_of(ET).assert_ok();
      for (size_t i = 0; i < count; ++i) {
        offset = align(offset, alignment);
        size_t num = classify(ET, subclasses, offset % 8);
        if (!num) return 0;
        for (size_t k = 0; k < num; ++k) {
          size_t pos = offset / 8;
          classes[k + pos] =
            merge_abi_classes(subclasses[k], classes[k + pos]);
        }
        offset += esize;
      }
      if (words > 2) {
        if (classes[0] != ABI_CLASS_SSE)
          return 0;
        for (size_t i = 1; i < words; ++i) {
          if (classes[i] != ABI_CLASS_SSEUP)
            return 0;
        }
      }
      for (size_t i = 0; i < words; i++) {
        if (classes[i] == ABI_CLASS_MEMORY)
          return 0;

        if (classes[i] == ABI_CLASS_SSEUP) {
          assert(i > 0);
          if (classes[i - 1] != ABI_CLASS_SSE
            && classes[i - 1] != ABI_CLASS_SSEUP) {
            classes[i] = ABI_CLASS_SSE;
          }
        }

        if (classes[i] == ABI_CLASS_X87UP) {
          assert(i > 0);
          if (classes[i - 1] != ABI_CLASS_X87) {
            return 0;
          }
        }
      }
      return words;
    }

    static size_t classify(const Type* T, ABIClass* classes, size_t offset) {
      switch (T->kind()) {
      case TK_Integer:
      case TK_Pointer: {
        size_t size = size_of(T).assert_ok() + offset;
        if (size <= 4) {
          classes[0] = ABI_CLASS_INTEGERSI;
          return 1;
        }
        else if (size <= 8) {
          classes[0] = ABI_CLASS_INTEGER;
          return 1;
        }
        else if (size <= 12) {
          classes[0] = ABI_CLASS_INTEGER;
          classes[1] = ABI_CLASS_INTEGERSI;
          return 2;
        }
        else if (size <= 16) {
          classes[0] = ABI_CLASS_INTEGER;
          classes[1] = ABI_CLASS_INTEGER;
          return 2;
        }
        else {
          return 0;
        }
      } break;
      case TK_Real: {
        size_t size = size_of(T).assert_ok();
        if (size == 4) {
          if (!(offset % 8))
            classes[0] = ABI_CLASS_SSESF;
          else
            classes[0] = ABI_CLASS_SSE;
          return 1;
        }
        else if (size == 8) {
          classes[0] = ABI_CLASS_SSEDF;
          return 1;
        }
        else {
          assert(false && "illegal type");
        }
      } break;
      case TK_Typename: {
        if (is_opaque(T)) {
          classes[0] = ABI_CLASS_NO_CLASS;
          return 1;
        }
        else {
          return classify(storage_type(T).assert_ok(), classes, offset);
        }
      } break;
      case TK_Vector: {
        auto tt = cast<VectorType>(T);
        return classify_array_like(size_of(T).assert_ok(),
          storage_type(tt->element_type).assert_ok(), tt->count(), classes, offset);
      } break;
      case TK_Array:
      case TK_Matrix: {
        auto tt = cast<ArrayLikeType>(T);
        return classify_array_like(size_of(T).assert_ok(),
          storage_type(tt->element_type).assert_ok(), tt->count(), classes, offset);
      } break;
      case TK_Tuple: {
        auto tt = cast<TupleType>(T);
        size_t count = tt->values.size();
        const Type* fields[count];
        for (size_t i = 0; i < tt->values.size(); ++i) {
          fields[i] = storage_type(tt->values[i]).assert_ok();
        }
        return classify_tuple_like(size_of(T).assert_ok(),
          fields, count, tt->packed, classes, offset);
      } break;
      default: {
        StyledStream ss;
        ss << "internal error: type " << T << " unsupported in ABI" << std::endl;
        assert(false && "not supported in ABI");
        return 0;
      } break;
      }
      return 0;
    }

    size_t classify(const Type* T, ABIClass* classes) {
      return classify(T, classes, 0);
    }
  }
} // namespace scopes::abi_windows_x64
