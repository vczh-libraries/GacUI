using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Parser
{
    enum Access
    {
        Public,
        Protected,
        Private,
    }

    class SymbolDecl
    {
        public Access Access { get; set; }
        public string Name { get; set; }
    }

    class NamespaceDecl : SymbolDecl
    {
        public List<SymbolDecl> Children { get; set; }
    }

    class TemplateDecl : SymbolDecl
    {
        public List<SymbolDecl> TypeParameters { get; set; }
        public SymbolDecl Element { get; set; }
    }

    class BaseTypeDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }
    }

    class ClassDecl : SymbolDecl
    {
        public List<BaseTypeDecl> BaseTypes { get; set; }
        public List<SymbolDecl> Children { get; set; }
    }

    class VarDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }
    }

    class FuncDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }
    }

    class EnumItemDecl : SymbolDecl
    {
    }

    class EnumDecl : SymbolDecl
    {
        public List<EnumItemDecl> Children { get; set; }
    }

    class TypedefDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }
    }
}
