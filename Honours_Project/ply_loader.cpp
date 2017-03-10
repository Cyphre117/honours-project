#include "ply_loader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "helpers.h"
#include <SDL.h>

PlyLoader::PlyLoader()
{

}

PlyLoader::~PlyLoader()
{

}

void PlyLoader::load( std::string filepath, std::vector<GLfloat>& data )
{
	// TODO: find out if we can just leave the file as always binary?
	//	- doesn't seem to be causing problems when loading text files.

	std::ifstream file( filepath, std::ios::binary );
	if( !file.good() )
	{
		std::cout << "ERROR: could not open '" << filepath << "'" << std::endl;
		return;
	}
	else
	{
		// Check the filesize
		file.seekg( 0, std::ios::end );
		size_t size = file.tellg();
		file.seekg( 0, std::ios::beg );

		std::cout << "Opened '" << filepath << "', size: " << size / 1024 << "KB" << std::endl;
	}
	
	// Construct a the vertex description by parsing the header
	std::string line;
	std::vector<VertexProperty> vertex_desc;
	bool binary_data = false;
	bool swap_endian = false;
	size_t num_verts = 0;

	// Check the first line is 'ply', so we actually have a .ply file
	std::getline( file, line );
	if( line != "ply" )
	{
		std::cout << "ERROR: unknown file type" << std::endl;
		return;
	}

	// Loop until we reach the end of the header
	while( line != "end_header" )
	{
		// Convert the std::string to a std::stringtream for convinience
		std::getline( file, line );
		std::stringstream stream( line );

		std::cout << ": " << line << std::endl;

		std::string identifier;
		stream >> identifier;

		if( identifier == "comment" || identifier == "end_header" )
		{
			// discard line
		}
		else if( identifier == "format" )
		{
			std::string type;
			stream >> type;

			if( type == "ascii" )
			{
				// The format is ASCII, we can continue as normal
			}
			else if( type == "binary_big_endian" )
			{
				swap_endian = true;
				binary_data = true;
			}
			else if( type == "binary_little_endian" )
			{
				binary_data = true;
			}
			else
			{
				std::cout << "ERROR: unkown data format: '" << type << "'" << std::endl;
			}
		}
		else if( identifier == "element" )
		{
			std::string type;
			stream >> type;
			if( type == "vertex" )
			{
				// The next number shoud be the number of verticies
				stream >> num_verts;
			}
		}
		else if( identifier == "property" )
		{
			std::string type;
			stream >> type;

			if( type == "list" )
			{
				// discard line
			}
			else if( type == "float" || type == "uchar" )
			{
				std::string name;
				stream >> name;

				// Set the size of the property in bytes
				unsigned char size = (type == "float" ? 4 : 1);

				PropertyIdent ident;
				switch( name[0] )
				{
				case 'x': ident = PropertyIdent::x; break;
				case 'y': ident = PropertyIdent::y; break;
				case 'z': ident = PropertyIdent::z; break;
				case 'r': ident = PropertyIdent::r; break;
				case 'g': ident = PropertyIdent::g; break;
				case 'b': ident = PropertyIdent::b; break;
				default: ident = PropertyIdent::discard; break;
				}

				vertex_desc.push_back( VertexProperty{ ident, size } );
			}
			else
			{
				std::cout << "ERROR: unknown property type '" << type << "' in '" << filepath << "'" << std::endl;
			}
		}
	} // Done parsing header
	
	// Print out the format for the verticies
	std::cout << "Format" << std::endl;
	for( auto& vd : vertex_desc )
	{
		std::cout << "\tType: " << toString( vd.type ) << ", size: " << (int)vd.size << std::endl;
	}

	/*
	if( binary_data )
	{
		// Repoen the file as binary
		size_t pos = file.tellg();					// Store the current position
		file.close();								// Close the ASCII file
		file.open( filepath, std::ios::binary );	// Reopen the file as binary
		file.seekg( pos, std::ios::beg );			// Restore the stream position

		// Ensure everything is fine
		if( !file.good() )
		{
			std::cout << "ERROR: could not reopen file as binary '" << filepath << "'" << std::endl;
			return;
		}
		else
		{
			std::cout << "Reading file '" << filepath << "' as binary from location Ox" << std::hex << std::uppercase << pos << std::dec << std::nouppercase << std::endl;
		}
	}*/

	// Prepare the data vector
	data.clear();
	data.reserve( num_verts * 6 );

	// Now we can actually read the data from the file
	for( size_t i = 0; i < num_verts && file.good(); i++ )
	{
		GLfloat x = 0, y = 0, z = 0;
		GLfloat r = 1, g = 1, b = 1;
		std::string discard;

		for( auto& vd : vertex_desc )
		{
			if( binary_data )
			{
				if( vd.size == 1 )
				{
					unsigned char byte;
					file.read( (char*)&byte, 1 );

					switch( vd.type )
					{
					case PropertyIdent::x: x = byte / 255.0f; break;
					case PropertyIdent::y: y = byte / 255.0f; break;
					case PropertyIdent::z: z = byte / 255.0f; break;
					case PropertyIdent::r: r = byte / 255.0f; break;
					case PropertyIdent::g: g = byte / 255.0f; break;
					case PropertyIdent::b: b = byte / 255.0f; break;
					case PropertyIdent::discard: break;
					}
				}
				else if( vd.size == 4 )
				{
					// WARNING: floats are hardcoded at 4 bytes in size
					// TODO: indicate somehow if floats are not infact 4 bytes, maybe crash nicely
					char byte_array[4] = { 0 };
					file.read( byte_array, sizeof( byte_array ) );

					if( swap_endian )
					{
						SDL_Swap32( *(Uint32*)byte_array );
					}
					
					switch( vd.type )
					{
					case PropertyIdent::x: x = *(float*)byte_array; break;
					case PropertyIdent::y: y = *(float*)byte_array; break;
					case PropertyIdent::z: z = *(float*)byte_array; break;
					case PropertyIdent::r: r = *(float*)byte_array; break;
					case PropertyIdent::g: g = *(float*)byte_array; break;
					case PropertyIdent::b: b = *(float*)byte_array; break;
					case PropertyIdent::discard: break;
					}
				}
				else
				{
					std::cout << "ERROR: cannot read binary poperty type with size " << vd.size << " bytes." << std::endl;
					break;
				}
			}
			else // ASCII data
			{
				// TODO: what if RGB values are a uchar,
				// do we need to divide by 255 when the file is ASCII? probably maybe?

				switch( vd.type )
				{
				case PropertyIdent::x: file >> x; break;
				case PropertyIdent::y: file >> y; break;
				case PropertyIdent::z: file >> z; break;
				case PropertyIdent::r: file >> r; break;
				case PropertyIdent::g: file >> g; break;
				case PropertyIdent::b: file >> b; break;
				case PropertyIdent::discard: file >> discard; break;
				}
			}
		}

		data.push_back( x );
		data.push_back( y );
		data.push_back( z );
		data.push_back( r );
		data.push_back( g );
		data.push_back( b );
	}

	// Check we actually read the correct number of verts
	if( data.size() / 6 == num_verts )
	{
		std::cout << "Read " << num_verts << " verticies" << std::endl;
	}
	else
	{
		std::cout << "ERROR: header indicated " << num_verts << " veticies, but we read " << data.size() / 6 << std::endl;
	}
}

std::string PlyLoader::toString( PropertyIdent p )
{
	switch( p )
	{
	case PropertyIdent::x: return "x";
	case PropertyIdent::y: return "y";
	case PropertyIdent::z: return "z";
	case PropertyIdent::r: return "r";
	case PropertyIdent::g: return "g";
	case PropertyIdent::b: return "b";
	case PropertyIdent::discard: return "discard";
	}
}