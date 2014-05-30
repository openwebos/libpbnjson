#include <iostream>
#include <pbnjson.hpp>

using namespace std;
using namespace pbnjson;

typedef pair<char const *, size_t> RegionT;

string directory;

class MyResolver
	: public JResolver
{
	virtual JSchema resolve(const ResolutionRequest &request,
	                        JSchemaResolutionResult &result)
	{
		string lookup_path = directory + "/" + request.resource() + ".schema";
		cout << "Resolving " << lookup_path << endl;
		if (-1 == ::access(lookup_path.c_str(), F_OK))
		{
			result = SCHEMA_NOT_FOUND;
			return JSchema::NullSchema();
		}

		JSchemaFile schema{lookup_path.c_str()};
		if (!schema.isInitialized())
		{
			result = SCHEMA_INVALID;
			return JSchema::NullSchema();
		}

		result = SCHEMA_RESOLVED;
		return schema;
	}
};

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		cerr << "Usage: LoadSchema <dir> <file>" << endl;
		return 1;
	}
	directory = argv[1];
	string schema_name = argv[2];

	MyResolver resolver;
	JSchemaFile schema{directory + "/" + schema_name, directory + "/" + schema_name, NULL, &resolver};
	if (!schema.isInitialized())
	{
		cerr << "Failed to initialize schema" << endl;
		return 1;
	}

	JDomParser parser{};
	parser.parse("{}", schema);

	return 0;
}
