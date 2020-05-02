/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "matrix_type.hpp"
#include "vector_type.hpp"
#include "typename_type.hpp"
#include "../error.hpp"
#include "../dyn_cast.inc"
#include "../hash.hpp"
#include "../utils.hpp"

#include <unordered_set>

namespace scopes {

namespace MatrixSet {
struct Hash {
    std::size_t operator()(const MatrixType *s) const {
        return
            hash2(
                std::hash<const Type *>{}(s->element_type),
                std::hash<size_t>{}(s->_count));
    }
};

struct KeyEqual {
    bool operator()( const MatrixType *lhs, const MatrixType *rhs ) const {
        return lhs->element_type == rhs->element_type
            && lhs->_count == rhs->_count;
    }
};
} // namespace MatrixSet

static std::unordered_set<const MatrixType *, MatrixSet::Hash, MatrixSet::KeyEqual> matrices;

//------------------------------------------------------------------------------
// MATRIX TYPE
//------------------------------------------------------------------------------

void MatrixType::stream_name(StyledStream &ss) const {
    ss << "<";
    stream_type_name(ss, element_type);
    ss << " * ";
    ss << _count;
    ss << ">";
}

const VectorType *MatrixType::column_type() const {
    return cast<VectorType>(storage_type(element_type).assert_ok());
}

MatrixType::MatrixType(const Type *_element_type, size_t _count)
    : ArrayLikeType(TK_Matrix, _element_type, _count) {
    auto ST = column_type();
    column_element_type = ST->element_type;
    row_count = ST->count();

    size = stride * count();
    align = align_of(element_type).assert_ok();
}

SCOPES_RESULT(const Type *) matrix_type(const Type *element_type, size_t count) {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_TYPE_KEY(MatrixType, key);
    key->element_type = element_type;
    key->_count = count;
    auto it = matrices.find(key);
    if (it != matrices.end())
        return *it;
    if (storage_kind(element_type) != TK_Vector) {
        SCOPES_ERROR(TypeKindMismatch, TK_Vector, element_type);
    }
    if (count < 2) {
        SCOPES_ERROR(InvalidMatrixSize);
    }
    auto result = new MatrixType(element_type, count);
    matrices.insert(result);
    return result;
}

} // namespace scopes
