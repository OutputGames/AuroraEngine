#include "shader.hpp"

#include "utils/math.hpp"

ShaderFactory::PropertyType ShaderFactory::GetTypeFromName(string n)
{
	if (n == "Float") return Float;
	if (n == "Int")	return Int;
	if (n == "Bool") return Bool;
	if (n == "Vector2") return Vector2;
	if (n == "Vector3") return Vector3;
	if (n == "Vector4") return Vector4;
	if (n == "Color") return Color;
	if (n == "Texture2D") return Texture2D;
	if (n == "TextureCubemap") return TextureCubemap;
	if (n == "Mat2") return Mat2;
	if (n == "Mat3") return Mat3;
	if (n == "Mat4") return Mat4;

	return Int;
}

void ShaderFactory::Property::GetRealValueFromDefault(string def)
{

	
			switch (GetType())
			{
			case Float:
			{
				defaultValueF = ((float)atof(def.c_str()));
				break;
			}
			case Int:
			{
				defaultValueI = (atoi(def.c_str()));
				break;
			}
			case Bool:
			{
				if (def == "True")
					defaultValueB = (true);
				else
					defaultValueB = (false);
				break;
			}
			case Vector2:
			{
				defaultValue2 = (Math::StringToVec2(def));
				break;
			}
			case Vector3:
			{
				defaultValue3 = (Math::StringToVec3(def));
				break;
			}
			case Vector4:
			{
				defaultValue4 = (Math::StringToVec4(def));
				break;
			}
			case Color:
				defaultValue4 = (Math::StringToVec4(def));
				break;
			default:
				defaultValueI = 0;
				break;
			}
			
}

ShaderFactory::ShaderData *ShaderFactory::LoadData(json j)
{
    string vdat = j["VertexCode"];
    string fdat = j["FragmentCode"];


	vdat = Filesystem::ReplaceOccurences(vdat, "; ", ";\n	");


    string vertex = Filesystem::ReplaceOccurences("", "_VERTEXCODEHERE_", vdat);

    cout << vertex;

    Filesystem::WriteFileString("C:/Users/chris/Downloads/testproj/Assets/testshad.txt", vertex);


    ShaderData* data = new ShaderData{ vertex, fdat };


    return data;
}

Shader::Shader(const char* path, bool overwrite)
{

	//string data = Filesystem::ReadFileString(path);

	ifstream input(path);

	bool foundShaderAlias = false;
	string shaderAlias;

	bool foundPropertyAlias = false;
	bool openedBrackets = false;
	bool closedBrackets = false;

	bool underVertexShader = false;
	bool underFragmentShader = false;

	string bracketProperty = "";

	vector<string> noBrackets = { "Properties", "Tags", "VertexShader", "FragmentShader" };

	string vertexShaderCode, fragmentShaderCode, geometryShaderCode;

	string baseVertex = "#version 330 core\n"
		"layout (location = 0) in vec3 INPUT_POSITION;\n"
		"layout (location = 1) in vec3 INPUT_NORMAL;\n"
		"layout (location = 2) in vec2 INPUT_TEXCOORDS;\n"
		"layout(location = 3) in ivec4 INPUT_BONEIDS; \n"
		"layout(location = 4) in vec4 INPUT_WEIGHTS;\n"
		"\n"
		"out vec2 OUTPUT_TEXCOORDS;\n"
		"out vec3 OUTPUT_POS;\n"
		"out vec3 OUTPUT_NORMAL;\n"
		"\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n"
		"\n"
		"// placeuniformshere\n"
		"\n"
		"const int MAX_BONES = 252;\n"
		"const int MAX_BONE_INFLUENCE = 4;\n"
		"uniform mat4 finalBonesMatrices[MAX_BONES];\n"
		"\n"
		"vec3 SAMPLE_POSITION(vec3 p) {\n"
		"\n"
		"    vec4 totalPosition = vec4(0.0f);\n"
		"    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)\n"
		"    {\n"
		"        if(INPUT_BONEIDS[i] == -1) \n"
		"            continue;\n"
		"        if(INPUT_BONEIDS[i] >=MAX_BONES) \n"
		"        {\n"
		"            totalPosition = vec4(p,1.0f);\n"
		"            break;\n"
		"        }\n"
		"        vec4 localPosition = finalBonesMatrices[INPUT_BONEIDS[i]] * vec4(p,1.0f);\n"
		"        totalPosition += localPosition * INPUT_WEIGHTS[i];\n"
		"        vec3 localNormal = mat3(finalBonesMatrices[INPUT_BONEIDS[i]]) * INPUT_NORMAL;\n"
		"   }\n"
		"\n"
		"    if (totalPosition == vec4(0.0))\n"
		"        totalPosition = vec4(p,1.0);\n"
		"\n"
		"    return vec3(model * totalPosition);\n"
		"}\n"
		"\n"
		"vec3 SAMPLE_NORMAL(vec3 n) {\n"
		"    return mat3(transpose(inverse(model))) * n; \n"
		"}\n"
		"\n"
		"vec4 TRANSFORMTOVIEW(vec3 p) {\n"
		"    return projection * view * vec4(p, 1.0);\n"
		"}\n"
		"";

	string baseFragment = "#version 330 core\n"
		"\n"
		"layout (location = 0) out vec4 DEFERRED_POS;\n"
		"layout (location = 1) out vec4 DEFERRED_NORMAL;\n"
		"layout (location = 2) out vec4 DEFERRED_COLOR;\n"
		"layout (location = 3) out vec4 DEFERRED_SHADING;\n"
		"\n"
		"in vec2 OUTPUT_TEXCOORDS;\n"
		"in vec3 OUTPUT_POS;\n"
		"in vec3 OUTPUT_NORMAL;\n"
		"\n"
		"// placeuniformshere\n"
		"";




	//baseVertex = Filesystem::ReplaceOccurences(baseVertex, "\n", "\n\t");

	vertexShaderCode += baseVertex;
	fragmentShaderCode += baseFragment;

	vector<string> bracketProperties;

	int lineCtr = 0;
	string lastLine;
	std::string line;
	while (std::getline(input, line))
	{

		line = Filesystem::ReplaceOccurences(line, "\t", "");

		//cout << line << endl;

		if (line.substr(0,2) == "//")
		{
			//cout << "found comment: " << line << endl;
			lastLine = line;
			// process pair (a,b)
			lineCtr++;
			continue;
		}

		if (line.length() <= 0)
		{
			lastLine = line;
			// process pair (a,b)
			lineCtr++;
			continue;
		}

		if (line.find("Shader") != string::npos && !foundShaderAlias)
		{
			int beginIdx = line.find("Shader \"")+8;
			int endIdx = 0;

			for (int i = beginIdx; i < line.length(); ++i)
			{
				char c = line[i];
				if (c == '\"')
				{
					endIdx = i-1;
					break;
				}
			}

			shaderAlias = line.substr(beginIdx, endIdx);

			//cout << "Found shader alias: " << shaderAlias << endl;

			foundShaderAlias = true;

		}

		if (line.find("Properties") != string::npos && !foundPropertyAlias)
		{
			foundPropertyAlias = false;
		}


			if (line.find("{") != string::npos)
			{
				closedBrackets = false;
				openedBrackets = true;

				if (line[line.length() - 1] == '{' && line.length() > 2) {
					lastLine = Filesystem::ReplaceOccurences(line, "{", "");
				}
				bracketProperty = lastLine;
				bracketProperties.push_back(bracketProperty);

				cout << "opening property " << bracketProperty << endl;

				if (find(noBrackets.begin(), noBrackets.end(), bracketProperty) != noBrackets.end()) {
					cout << "	this is a nobracket" << endl;
					lastLine = line;
					// process pair (a,b)
					lineCtr++;
					continue;
				}

				//cout << "Opened bracket " << bracketProperty << endl;
			}

		if (line.find("}") != string::npos)
		{
			openedBrackets = false;
			closedBrackets = true;

			//cout << "Closed bracket: " << bracketProperty << endl;

			if (bracketProperty == "VertexShader")
			{
				underVertexShader = false;
			}

			if (bracketProperty == "FragmentShader")
			{
				underFragmentShader = false;
			}

			cout << "closing property " << bracketProperty << endl;

			if (bracketProperties.size() > 1) {
				bracketProperty = bracketProperties[bracketProperties.size() - 2];
				bracketProperties.erase(bracketProperties.begin() + (bracketProperties.size() - 1));
			}

		}

		if (openedBrackets)
		{



			if (bracketProperty == "Properties")
			{
				//cout << "found property line " << line << endl;

				if (line.find('(') != string::npos)
				{

					// Find property data

					int dataBegin = line.find("(")+1;
					int dataEnd = line.find(")")-(dataBegin-1);

					string prDat = line.substr(dataBegin, dataEnd);
					prDat = Filesystem::ReplaceOccurences(prDat, "\"", "");

					int splitter = prDat.find(",");

					string propertyName = prDat.substr(0, splitter);
					string propertyType = prDat.substr(splitter+1, prDat.length() - (splitter+1));

					if (propertyType[0] == ' ')
					{
						propertyType = propertyType.substr(1, propertyType.length() - 1);
					}

					if (propertyType[propertyType.length()-1] == ')')
					{
						propertyType = propertyType.substr(0, propertyType.length() - 1);
					}

					// Find property defaults

					int equalBeg = line.find("=");

					string defDat = line.substr(equalBeg+1, (line.length())-equalBeg);

					defDat = Filesystem::ReplaceOccurences(defDat, " ", "");
					defDat = Filesystem::ReplaceOccurences(defDat, "(", "");
					defDat = Filesystem::ReplaceOccurences(defDat, ")", "");

					// Find property ID

					string propertyId = line.substr(0, dataBegin-1);

					propertyId = Filesystem::ReplaceOccurences(propertyId, " ", "");

					// Process this data

					//ShaderFactory::PropertyType type = ShaderFactory::GetTypeFromName(propertyType);

					ShaderFactory::Property* p = new ShaderFactory::Property;
					p->name = propertyName;
					p->type = propertyType;
					p->GetRealValueFromDefault(defDat);
					properties.insert({ propertyId, p });

					/*
					switch (type)
					{
					case ShaderFactory::Float:
					{
						ShaderFactory::Property<float> p;
						p.GetRealValueFromDefault(defDat, type);
						p.name = propertyName;
						p.type = propertyType;
						properties.insert({ propertyId, &p });
					}
					case ShaderFactory::Int:
					{
						ShaderFactory::Property<int> p;
						p.GetRealValueFromDefault(defDat, type);
						p.name = propertyName;
						p.type = propertyType;
						properties.insert({ propertyId, &p });
					}
					case ShaderFactory::Bool:
					{
						ShaderFactory::Property<bool> p;
						p.GetRealValueFromDefault(defDat, type);
						p.name = propertyName;
						p.type = propertyType;
						properties.insert({ propertyId, &p });
					}
					case ShaderFactory::Vector2:
					{
						ShaderFactory::Property<vec2> p;
						p.GetRealValueFromDefault(defDat, type);
						p.name = propertyName;
						p.type = propertyType;
						properties.insert({ propertyId, &p });
					}
					case ShaderFactory::Vector3:
					{
						ShaderFactory::Property<vec3> p;
						p.GetRealValueFromDefault(defDat, type);
						p.name = propertyName;
						p.type = propertyType;
						properties.insert({ propertyId, &p });
					}
					case ShaderFactory::Vector4:
					{
						ShaderFactory::Property<vec4> p;
						p.GetRealValueFromDefault(defDat, type);
						p.name = propertyName;
						p.type = propertyType;
						properties.insert({ propertyId, &p });
					}
					default:
						break;
					}
					*/


					//cout << "	property id: " << propertyId << endl;
					//cout << "	property name: " << propertyName << endl;
					//cout << "	property type: " << propertyType << endl;
					//cout << "	property default: " << defDat << endl;
						
				}

			}

			if (bracketProperty == "Tags")
			{
				line = Filesystem::ReplaceOccurences(line, "\"", "");
				line = Filesystem::ReplaceOccurences(line, " ", "");
				//cout << "found tag line " << line << endl;
				if (line.find("=") != string::npos)
				{

					int splitter = line.find("=");

					string preSplit = line.substr(0, splitter);
					string postSplit = line.substr((splitter+1), line.length() - (splitter+1));

					string tagName = preSplit;
					string tagVal = postSplit;
						
					//cout << "\ttag name: " << tagName << endl;
					//cout << "\ttag value: " << tagVal << endl;

					tags.insert({ tagName, tagVal });
				}
			}

			if (bracketProperty == "VertexShader")
			{
				underVertexShader = true;
			}


			if (bracketProperty == "FragmentShader")
			{
				underFragmentShader = true;
			}

		}

		if (underVertexShader)
		{
			//line = Filesystem::ReplaceOccurences(line, "\t", "");

			line = Filesystem::ReplaceOccurences(line, "RENDERER_POSITION", "gl_Position");

			cout << line << endl;
			vertexShaderCode += line + "\n";
		}

		if (underFragmentShader)
		{
			cout << line << endl;
			fragmentShaderCode += line + "\n";
		}


		lastLine = line;
		// process pair (a,b)
		lineCtr++;
	}


	string uniformString = "// placeuniformshere\n";
	int vsUniformIndex = vertexShaderCode.find(uniformString)+uniformString.size();
	int fsUniformIndex = fragmentShaderCode.find(uniformString) + uniformString.size();

	for (pair<const string, ShaderFactory::Property*> value : properties)
	{
		string unifString = "uniform ";

		switch (value.second->GetType())
		{
		case ShaderFactory::Float:
			unifString += "float";
			break;
		case ShaderFactory::Int:
			unifString += "int";
			break;
		case ShaderFactory::Bool:
			unifString += "bool";
			break;
		case ShaderFactory::Vector2:
			unifString += "vec2";
			break;
		case ShaderFactory::Vector3:
			unifString += "vec3";
			break;
		case ShaderFactory::Vector4:
			unifString += "vec4";
			break;
		case ShaderFactory::Color:
			unifString += "vec4";
			break;
		case ShaderFactory::Texture2D:
			unifString += "sampler2D";
			break;
		case ShaderFactory::TextureCubemap:
			unifString += "samplerCube";
			break;
		}

		unifString += " ";
		unifString += value.first;
			unifString += ";\n";

		vertexShaderCode = vertexShaderCode.insert(vsUniformIndex, unifString);
		fragmentShaderCode = fragmentShaderCode.insert(fsUniformIndex, unifString);
	}

	vertexShaderCode = Filesystem::ReplaceOccurences(vertexShaderCode, uniformString, "");
	fragmentShaderCode = Filesystem::ReplaceOccurences(fragmentShaderCode, uniformString, "");

	cout << "Compiled shader sucessfully." << endl;

	cout << "--- SHADER RESULTS ---" << endl;

	cout << "\tShader Alias: " << shaderAlias << endl;

	cout << "\t--- Properties ---" << endl;
	for (auto& [propertyId, property] : properties)
	{
		ShaderFactory::PropertyType type = property->GetType();

		cout << "\t\tProperty Id: " << propertyId << endl;
		cout << "\t\t\tProperty Name: " << property->name << endl;
		cout << "\t\t\tProperty Type: " << property->type << endl;

		ShaderFactory::Property* p = property;

		cout << "\t\t\tProperty Default: ";
		switch (type)
		{
		case ShaderFactory::Float:
		{
			cout << p->defaultValueF << endl;
			break;
		}
		case ShaderFactory::Int:
		{
			cout << p->defaultValueI << endl;
			break;
		}
		case ShaderFactory::Bool:
		{
			cout << p->defaultValueB << endl;
			break;
		}
		case ShaderFactory::Vector2:
		{
			cout << p->defaultValue2.x << "," << p->defaultValue2.y << endl;
			break;
		}
		case ShaderFactory::Vector3:
		{
			cout << p->defaultValue3.x << "," << p->defaultValue3.y << "," << p->defaultValue3.z << endl;
			break;
		}
		case ShaderFactory::Vector4:
		{
			cout << p->defaultValue4.x << "," << p->defaultValue4.y << "," << p->defaultValue4.z << "," << p->defaultValue4.w << endl;
			break;
		}
		case ShaderFactory::Color:
		{
			cout << p->defaultValue4.x << "," << p->defaultValue4.y << "," << p->defaultValue4.z << "," << p->defaultValue4.w << endl;
			break;
		}
		default:
			//cout << "not implemented yet." << endl;
			break;
		}

	}

	cout << "\t--- Tags ---" << endl;
	for (pair<const string, string> tag : tags)
	{
		cout << "\t\t" << tag.first << " is " << tag.second << endl;
	}

	cout << "\t--- Vertex Shader ---" << endl;

	//cout << vertexShaderCode << endl;

	cout << "\t--- Fragment Shader ---" << endl;

	//cout << fragmentShaderCode << endl;

	useGeometry = false;
	shaderDirectory = path;
	name = shaderAlias;
	isFactoryShader = true;

	loadSource(vertexShaderCode, fragmentShaderCode, geometryShaderCode);


	loadedShaders[name] = this;

	Filesystem::WriteFileString("C:/Users/chris/Downloads/testproj/Assets/testvtx.txt", vertexShaderCode);
	Filesystem::WriteFileString("C:/Users/chris/Downloads/testproj/Assets/testfrg.txt", fragmentShaderCode);

}

string Shader::CreateShader()
{

	string s =
		"Shader \"Custom/NewShader\"\n"
		"{\n"
		"\tProperties\n"
		"\t{\n"
		"\t\t// Properties go here\n"
		"\n"
		"\t\t_BaseColor(\"Base Color\", Color) = (1, 1, 1)\n"
		"                _Roughness(\"Roughness\", Float) = 0.5\n"
		"                _Metallic(\"Metallic\", Float) = 0.0\n"
		"                _Alpha(\"Alpha\", Float) = 1.0\n"
		"                _ColorMap(\"Color Map\", Texture2D) = 0\n"
		"\t}\n"
		"\n"
		"\tTags\n"
		"\t{\n"
		"\t\t// Tags go here\n"
		"                \"RenderPass\" = \"Deferred\"\n"
		"\t}\n"
		"\n"
		"\tVertexShader\n"
		"\t{\n"
		"                void main() \n"
		"                {\n"
		"                        // Vertex Shader Code goes here\n"
		"\n"
		"                        OUTPUT_TEXCOORDS = INPUT_TEXCOORDS;\n"
		"                        OUTPUT_POS = SAMPLE_POSITION(INPUT_POSITION);\n"
		"                        OUTPUT_NORMAL = SAMPLE_NORMAL(INPUT_NORMAL);\n"
		"\n"
		"                        RENDERER_POSITION = TRANSFORMTOVIEW(OUTPUT_POS);\n"
		"                }\n"
		"\t}\n"
		"\n"
		"\tFragmentShader\n"
		"\t{\n"
		"                void main()\n"
		"                {\n"
		"                                // Fragment Shader Code goes here\n"
		"                        DEFERRED_POS = vec4(OUTPUT_POS,_Alpha);\n"
		"                        DEFERRED_NORMAL = vec4(normalize(OUTPUT_NORMAL),_Alpha);\n"
		"\n"
		"                        vec3 diffuse = texture(_ColorMap, OUTPUT_TEXCOORDS).rgb;\n"
		"\n"
		"                        diffuse *= _BaseColor;\n"
		"\n"
		"                        DEFERRED_COLOR = vec4(diffuse, _Alpha);\n"
		"\n"
		"                        DEFERRED_SHADING = vec4(_Roughness, _Metallic, 1.0,_Alpha);\n"
		"                }\n"
		"\t}\n"
		"\n"
		"}\n"
		"";

	return s;

}
