Architecture
============

Scopes understands and transforms source code in multiple cleanly separated stages:

Order |Stage       |From                    |To
------|------------|------------------------|----------------------
1     |Parsing     |Data Interchange Format |S-Expression Tree
2     |Expansion   |S-Expression Tree       |Untyped Scopes AST
3     |Checking    |Untyped Scopes AST      |Typed Scopes IL
4     |Translation |Typed Scopes IL         |LLVM IR / SPIR-V
5     |Execution   |LLVM IR / SPIR-V        |Program Output


TODO