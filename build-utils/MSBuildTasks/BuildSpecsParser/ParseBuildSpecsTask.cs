using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace BuildSpecsParser
{
    public class ParseBuildSpecsTask : Task
    {
        public const string Format =
@"[
    {
        ""solutionName"": {
            ""property1"": ""value"",
            ""property2"": ""value2"",
            ...
        }
    },
    {
        ""solution2name"": { ... }
    },
    ...
]";
        public override bool Execute()
        {
            try
            {
                var solutionsArray = JArray.Parse(JsonBuildSpecs);

                List<BuildSpec> specs = new List<BuildSpec>();
                foreach (var solution in solutionsArray)
                {
                    if (solution.Type != JTokenType.Object)
                    {
                        throw new Exception("Each entry in the solutions array must be an object.");
                    }
                    var solutionObjProps = ((JObject)solution).Properties();
                    if (solutionObjProps.Count() != 1)
                    {
                        throw new Exception("Solution object must have exactly one key (the solution's filename)");
                    }
                    if (solutionObjProps.First().Value.Type != JTokenType.Object)
                    {
                        throw new Exception("The value of the solution name's key must be an object");
                    }

                    var spec = new BuildSpec();
                    spec.SolutionName = ParseSpecialTokens(solutionObjProps.First().Name);

                    foreach (var property in ((JObject)solutionObjProps.First().Value).Properties())
                    {
                        if (property.Value.Type != JTokenType.String)
                        {
                            throw new Exception($"In solution '{spec.SolutionName}': The value of the property '{property.Name}' must be a string");
                        }
                        spec.Properties[ParseSpecialTokens(property.Name)] = ParseSpecialTokens((string)property.Value);
                    }
                    specs.Add(spec);
                }

                ParsedBuildSpecs = specs.Select(spec =>
                {
                    return new TaskItem(spec.SolutionName, new Dictionary<string, string>()
                    {
                        {
                            "AdditionalProperties", spec.Properties.Aggregate("", (propList, propKvp) =>
                            {
                                propList += $"{propKvp.Key}={propKvp.Value};";
                                return propList;
                            })
                        }
                    });
                }).ToArray();
            }
            catch (Exception ex)
            {
                Log.LogError("An error occurred while parsing Build Specs: " + ex.Message);
                Log.LogError("Make sure your Build Specs follow the format: \n" + Format);
                return false;
            }

            return true;
        }

        private string ParseSpecialTokens(string inputString)
        {
            return inputString.Replace("{workingDir}", System.IO.Directory.GetCurrentDirectory());
        }

        [Required]
        public string JsonBuildSpecs { get; set; }

        [Output]
        public ITaskItem[] ParsedBuildSpecs { get; set; }
    }
}
