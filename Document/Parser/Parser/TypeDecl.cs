using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Parser
{
    class TypeDecl
    {
    }

    class RefTypeDecl : TypeDecl
    {
        public string Name { get; set; }
    }

    class SubTypeDecl : TypeDecl
    {
        public TypeDecl Parent { get; set; }
        public string Name { get; set; }
    }

    enum Decoration
    {
        Const,
        Volatile,
        Reference,
        Pointer,
    }

    class DecorateTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public Decoration Decoration { get; set; }
    }

    class ArrayTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public int? Size { get; set; }
    }

    enum CallingConvention
    {
        CDecl,
        ClrCall,
        StdCall,
        FastCall,
        ThisCall,
        VectorCall,
    }
    class FunctionTypeDecl : TypeDecl
    {
        public CallingConvention CallingConvention { get; set; }
        public TypeDecl ReturnType { get; set; }
        public TypeDecl ClassType { get; set; }
        public List<VarDecl> Parameters { get; set; }
    }
}
