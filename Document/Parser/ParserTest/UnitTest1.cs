using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Parser;

namespace ParserTest
{
    [TestClass]
    public class TypeDeclTest
    {
        static string Convert(string input)
        {
            var tokens = input.Split(" ".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
            int index = 0;
            TypeDecl decl = null;
            string name = null;
            Assert.IsTrue(TypeDecl.ParseType(tokens, ref index, out decl, out name));

            if (name == null)
            {
                return decl.ToString();
            }
            else
            {
                return name + " : " + decl.ToString();
            }
        }

        [TestMethod]
        public void TestSimpleType()
        {
            Assert.AreEqual(
                "int",
                Convert("int")
                );
        }

        [TestMethod]
        public void TestDecoratedType()
        {
            Assert.AreEqual(
                "const int",
                Convert("const int")
                );
            Assert.AreEqual(
                "const int",
                Convert("int const")
                );

            Assert.AreEqual(
                "volatile int",
                Convert("volatile int")
                );
            Assert.AreEqual(
                "volatile int",
                Convert("int volatile")
                );

            Assert.AreEqual(
                "signed int",
                Convert("signed int")
                );
            Assert.AreEqual(
                "unsigned int",
                Convert("unsigned int")
                );
            Assert.AreEqual(
                "* int",
                Convert("int *")
                );
            Assert.AreEqual(
                "& int",
                Convert("int &")
                );
            Assert.AreEqual(
                "&& int",
                Convert("int &&")
                );
        }
    }
}
