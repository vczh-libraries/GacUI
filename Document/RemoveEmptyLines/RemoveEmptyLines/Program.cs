using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RemoveEmptyLines
{
    class Program
    {
        static void Main(string[] args)
        {
            var lines = File.ReadAllLines(args[0]);
            using (var writer = new StreamWriter(args[0], false, Encoding.UTF8))
            {
                foreach (var line in lines)
                {
                    if (!string.IsNullOrWhiteSpace(line))
                    {
                        writer.WriteLine(line);
                    }
                }
            }
        }
    }
}
