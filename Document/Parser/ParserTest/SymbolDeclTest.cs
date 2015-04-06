using System;
using System.Text;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Parser;

namespace ParserTest
{
    [TestClass]
    public class SymbolDeclTest
    {
        static SymbolDecl Convert(string input)
        {
            var tokens = input.Split(" ".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
            int index = 0;

            var decl = new SymbolDecl();
            SymbolDecl.ParseSymbols(tokens, ref index, decl);
            Assert.AreEqual(tokens.Length, index);

            return decl;
        }

        [TestMethod]
        public void TestNamespace()
        {
            var global = Convert("namespace vl { }");
            Assert.AreEqual(1, global.Children.Count);
            Assert.AreEqual("namespace vl", global.Children[0].ToString());
        }

        [TestMethod]
        public void TestUsingNamespace()
        {
            Assert.IsTrue(false);
        }

        [TestMethod]
        public void TestClass()
        {
            Assert.IsTrue(false);
        }

        [TestMethod]
        public void TestFriendClass()
        {
            Assert.IsTrue(false);
        }

        [TestMethod]
        public void TestFriendFunction()
        {
            Assert.IsTrue(false);
        }

        [TestMethod]
        public void TestUsingFunction()
        {
            Assert.IsTrue(false);
        }

        [TestMethod]
        public void TestClassField()
        {
            Assert.IsTrue(false);
        }

        [TestMethod]
        public void TestClassFunction()
        {
            Assert.IsTrue(false);
        }

        [TestMethod]
        public void TestEnum()
        {
            Assert.IsTrue(false);
        }

        [TestMethod]
        public void TestTypedefUsing()
        {
            Assert.IsTrue(false);
        }

        [TestMethod]
        public void TestTemplate()
        {
            Assert.IsTrue(false);
        }
    }
}
