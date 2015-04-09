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
            var regex = new Regex(@"^#line\s\d+\s""(?<path>[^""]*)""$");
            var lines = File.ReadAllLines(args[0]);
            var referenceFolder = Path.GetFullPath(Path.GetDirectoryName(args[1])).ToUpper();
            if (!referenceFolder.EndsWith("\\")) referenceFolder += "\\";
            var nonSystemSourceFolder = Path.GetFullPath(args[2]).ToUpper();
            if (!nonSystemSourceFolder.EndsWith("\\")) nonSystemSourceFolder += "\\";
            bool keep = true;
            using (var writer = new StreamWriter(args[0], false, Encoding.UTF8))
            {
                foreach (var line in lines)
                {
                    var match = regex.Match(line);
                    if (match.Success)
                    {
                        var path = match.Groups["path"].Value.Replace("\\\\", "\\").ToUpper();
                        if (path.Length >= 2 && path[1] != ':')
                        {
                            path = Path.GetFullPath(referenceFolder + path).ToUpper();
                        }
                        keep = path.StartsWith(nonSystemSourceFolder);
                    }
                    else if (keep)
                    {
                        writer.WriteLine(line);
                    }
                }
            }
        }
    }
}
