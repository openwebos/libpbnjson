// @@@LICENSE
//
// Copyright (c) 2014 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LICENSE@@@


#include <iostream>
#include <sys/ioctl.h>
#include <pbnjson.hpp>
#include <boost/program_options.hpp>

using namespace std;
using namespace pbnjson;

namespace {

const char *Basename(const char *path)
{
	const char *res = strrchr(path, '/');
	return res ? res + 1 : path;
}

int DetectTerminalWidth()
{
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	return w.ws_col;
}

class SchemaErrorHandler
	: public JErrorHandler
{
private:
	virtual void syntax(JParser *, SyntaxError code, const string &reason)
	{
		cerr << "Syntax error " << code << ": " << reason << endl;
	}

	virtual void schema(JParser *, SchemaError code, const string &reason)
	{
		cerr << "Schema error " << code << ": " << reason << endl;
	}

	virtual void misc(JParser *, const string &reason)
	{
		cerr << "Unknown error: " << reason << endl;
	}

	virtual void parseFailed(JParser *, const string &reason)
	{
		cerr << "Parse failed: " << reason << endl;
	}
};

} //namespace;

int main(int argc, char *argv[])
{
	const char *program_name = Basename(argv[0]);
	int line_length = DetectTerminalWidth();

	string file_name;
	string schema_file;

	try
	{
		using namespace boost::program_options;
		options_description desc("Options", line_length, line_length / 2);
		desc.add_options()
			("version,V", "Print program version")
			("help,h", "Print usage summary")
			("file,f", value<string>(&file_name)->default_value(file_name),
			 "JSON file to validate (skip for stdin)")
			("schema,s", value<string>(&schema_file)->default_value(schema_file),
			 "File with JSON schema")
			;

		positional_options_description p;
		p.add("file", 1);

		variables_map vm;
		store(command_line_parser(argc, argv)
		      .options(desc)
		      .positional(p)
		      .run(),
		      vm);
		notify(vm);

		if (vm.count("help"))
		{
			cout << program_name << " -- validate a JSON file against JSON schema\n\n";
			cout << "Usage: " << program_name << " [OPTION] <file.json>\n";
			cout << desc << endl;
			return 0;
		}

		if (vm.count("version"))
		{
			cout << program_name << " " << WEBOS_COMPONENT_VERSION << endl;
			return 0;
		}

		SchemaErrorHandler error_handler;

		// Prepare JSON schema
		unique_ptr<JSchema> schema;
		if (schema_file.empty())
			schema.reset(new JSchemaFragment("{}"));
		else
		{
			schema.reset(new JSchemaFile(schema_file, &error_handler));
			if (!schema->isInitialized())
			{
				cerr << "Failed to open JSON schema " << schema_file << endl;
				return 1;
			}
		}

		// Try to parse the file to validate
		JDomParser parser;
		if (file_name.empty())
		{
			if (!parser.begin(*schema.get(), &error_handler))
			{
				cerr << "Failed to parse JSON stdin" << endl;
				return 1;
			}

			string buf;
			while (getline(cin, buf))
			{
				if (!parser.feed(buf))
				{
					cerr << "Failed to parse JSON stdin" << endl;
					return 1;
				}
			}

			if (!parser.end())
			{
				cerr << "Failed to finalyze parse" << endl;
				return 1;
			}
		}
		else
		{
			// Try to parse the file
			if (!parser.parseFile(file_name, *schema.get(), JFileOptNoOpt, &error_handler))
			{
				cerr << "Failed to parse JSON file " << file_name << endl;
				return 1;
			}
		}
	}
	catch (const std::exception &e)
	{
		cerr << e.what() << endl;
		return 1;
	}
	return 0;
}
