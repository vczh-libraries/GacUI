using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Preprocessor
{
    class Program
    {
        static void Main(string[] args)
        {
            var inputPath = args[1];
            var outputPath = args[2];
            var regexSystem = new Regex(@"0x[a-f0-9]+ <\w+:\\");
            var regexLocal = new Regex(@"0x[a-f0-9]+ <" + Regex.Escape(args[0] + "/"));

            bool keep = false;
            using (var input = new StreamReader(inputPath))
            using (var output = new StreamWriter(outputPath, false, Encoding.UTF8))
            {
                while (!input.EndOfStream)
                {
                    var line = input.ReadLine();
                    if (line.Contains("<.Output"))
                    {
                        int a = 0;
                    }
                    if (regexSystem.IsMatch(line))
                    {
                        keep = false;
                    }
                    else if (regexLocal.IsMatch(line))
                    {
                        keep = true;
                    }
                    if (keep)
                    {
                        output.WriteLine(line);
                    }
                }
            }
        }
    }
}
