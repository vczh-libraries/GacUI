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
    }
}
