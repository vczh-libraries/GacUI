using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace RemoveSystemInclude
{
    class Program
    {
        static void Main(string[] args)
        {
            var regex = new Regex(@"^\s*#include\s*<[^\>]*>\s*$");
            var lines = File.ReadAllLines(args[0]);
            using (var writer = new StreamWriter(args[0], false, Encoding.UTF8))
            {
                foreach(var line in lines)
                {
                    if (!regex.IsMatch(line))
                    {
                        writer.WriteLine(line);
                    }
                }
            }
        }
    }
}
