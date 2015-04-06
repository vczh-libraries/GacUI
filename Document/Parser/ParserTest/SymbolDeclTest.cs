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
            var tokens = input.Split(" \r\n".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
            int index = 0;

            var decl = new SymbolDecl();
            SymbolDecl.ParseSymbols(tokens, ref index, decl);
            Assert.AreEqual(tokens.Length, index);

            return decl;
        }

        [TestMethod]
        public void TestNamespace()
        {
            var global = Convert(@"
                namespace vl { }
                ");
            Assert.AreEqual(1, global.Children.Count);
            Assert.AreEqual("namespace vl", global.Children[0].ToString());
        }

        [TestMethod]
        public void TestUsingNamespace()
        {
            var global = Convert(@"
                using namespace vl ;
                using namespace vl : : collections ;
                using namespace vl : : parsing : : xml ;
                ");
            Assert.AreEqual(3, global.Children.Count);
            Assert.AreEqual("using namespace vl", global.Children[0].ToString());
            Assert.AreEqual("using namespace vl::collections", global.Children[1].ToString());
            Assert.AreEqual("using namespace vl::parsing::xml", global.Children[2].ToString());
        }

        [TestMethod]
        public void TestClass()
        {
            Assert.IsTrue(false);
        }

        [TestMethod]
        public void TestFriendClass()
        {
            var global = Convert(@"
                friend class Class ;
                friend class Outer : : Inner ;
                friend class typename List < T > : : Inner ;
                friend class typename List < T > : : template Inner < T > ;
                template < typename T > friend class Ptr ;
                ");
            Assert.AreEqual(0, global.Children.Count);
        }

        [TestMethod]
        public void TestFriendFunction()
        {
            var global = Convert(@"
                friend void Func ( void ) ;
                friend int Class : : Func ( int ) ;
                template < typename T > friend T Class : : Func ( T ) ;
                ");
            Assert.AreEqual(0, global.Children.Count);
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
