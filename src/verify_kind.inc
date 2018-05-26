/*
Scopes Compiler
Copyright (c) 2016, 2017, 2018 Leonard Ritter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

namespace scopes {

template<TypeKind tk>
inline void verify_kind(const Type *T) {
    if (T->kind() != tk) {
        StyledString ss;
        ss.out << "value of ";
        switch(tk) {
        case TK_Integer: ss.out << "integer"; break;
        case TK_Real: ss.out << "real"; break;
        case TK_Pointer: ss.out << "pointer"; break;
        case TK_Array: ss.out << "array"; break;
        case TK_Vector: ss.out << "vector"; break;
        case TK_Tuple: ss.out << "tuple"; break;
        case TK_Union: ss.out << "union"; break;
        case TK_Typename: ss.out << "typename"; break;
        case TK_ReturnLabel: ss.out << "return label"; break;
        case TK_Function: ss.out << "function"; break;
        case TK_Extern: ss.out << "extern"; break;
        case TK_Image: ss.out << "image"; break;
        case TK_SampledImage: ss.out << "sampled image"; break;
        }
        ss.out << " kind expected, got " << T;
        location_error(ss.str());
    }
}

} // namespace scopes
