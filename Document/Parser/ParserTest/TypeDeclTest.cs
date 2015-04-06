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

        [TestMethod]
        public void TestDecoratedTypeProperity()
        {
            Assert.AreEqual(
                "* const int",
                Convert("const int *")
                );
            Assert.AreEqual(
                "const * int",
                Convert("int * const")
                );
            Assert.AreEqual(
                "const * const int",
                Convert("const int * const")
                );
        }

        [TestMethod]
        public void TestGenericType()
        {
            Assert.AreEqual(
                "vector<int>",
                Convert("vector < int >")
                );
            Assert.AreEqual(
                "vector<int, float>",
                Convert("vector < int , float >")
                );
        }

        [TestMethod]
        public void TestSubType()
        {
            Assert.AreEqual(
                "Outer::Inner",
                Convert("Outer : : Inner")
                );
            Assert.AreEqual(
                "Outer::Inner::MostInner",
                Convert("Outer : : Inner : : MostInner")
                );
        }

        [TestMethod]
        public void TestClassMemberType()
        {
            Assert.AreEqual(
                "Class::(int)",
                Convert("int Class : :")
                );
        }

        [TestMethod]
        public void TestArrayType()
        {
            Assert.AreEqual(
                "array int",
                Convert("int [ ]")
                );
            Assert.AreEqual(
                "array int",
                Convert("int [ 10 ]")
                );
            Assert.AreEqual(
                "array array int",
                Convert("int [ 10 ] [ 20 ]")
                );
            Assert.AreEqual(
                "array array int",
                Convert("int [ sizeof ( double ) ] [ call ( 1 , 2 , 3 ) ]")
                );
        }

        [TestMethod]
        public void TestFunctionType()
        {
            Assert.AreEqual(
                "function (a : int, float, double) : int",
                Convert("int ( int a , float , double )")
                );
            Assert.AreEqual(
                "function __stdcall (a : int, float, double) : int",
                Convert("int __stdcall ( int a , float , double )")
                );
            Assert.AreEqual(
                "* Class::(function __thiscall const (a : int, float, double) : int)",
                Convert("int ( __thiscall Class : : * ) ( int a , float , double ) const")
                );
            Assert.AreEqual(
                "callback : * Class::(function __thiscall (a : int, float, double) : int)",
                Convert("int ( __thiscall Class : : * callback ) ( int a , float , double )")
                );
        }
    }
}
