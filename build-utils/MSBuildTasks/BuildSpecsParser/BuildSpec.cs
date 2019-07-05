using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Text;

namespace BuildSpecsParser
{
    class BuildSpec
    {
        public string SolutionName { get; set; }

        public Dictionary<string, string> Properties { get; } = new Dictionary<string, string>();
    }
}
