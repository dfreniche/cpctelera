#ifndef _OUTPUT_GENERATOR_H_
#define _OUTPUT_GENERATOR_H_

#include <vector>
#include <fstream>
#include <iomanip>
#include "ConversionOptions.hpp"
#include "Tile.hpp"
#include "FileUtils.hpp"

using namespace std;

class OutputGenerator {

	vector<int> GetPaletteValues(ConversionOptions &options) {
		vector<int> palette;
		switch (options.PaletteFormat) {
		case ConversionOptions::FIRMWARE:
			palette = options.Palette.GetPaletteAsFW();
			break;
		case ConversionOptions::HARDWARE:
			palette = options.Palette.GetPaletteAsHW();
			break;
		default:
			break;
		}
		return palette;
	};

public:
	string toHexString(unsigned char data) {
		stringstream ss;
		ss << hex << setw(2) << setfill('0') << (int)data;
		return ss.str();
	};

	void DumpPaletteASM(ConversionOptions &options, ofstream &ofs) {
		vector<int> palette = GetPaletteValues(options);
		unsigned int numColors = palette.size();
		if (numColors > 0) {
			ofs << "; Palette uses " << ConversionOptions::ToString(options.PaletteFormat) << " values." << endl;
			
			if(!options.BaseName.empty()) {
				ofs << options.BaseName << "_";
			}
			ofs << "palette:" << endl << "DEFB ";
			
			for (unsigned int i = 0; i<numColors; ++i) {
				if (i > 0) {
					ofs << ", ";
				}
				ofs << "#" << toHexString(palette[i]);
			}
			ofs << endl << endl;
		}
	}

	void DumpPaletteASXXXX(ConversionOptions &options, ofstream &ofs) {
		vector<int> palette = GetPaletteValues(options);
		unsigned int numColors = palette.size();
		if (numColors > 0) {
			ofs << "; Palette uses " << ConversionOptions::ToString(options.PaletteFormat) << " values." << endl;
			
			if(!options.BaseName.empty()) {
				ofs << "_" << options.BaseName << "_";
			}
			ofs << "palette::" << endl << ".db ";

			for (unsigned int i = 0; i<numColors; ++i) {
				if (i > 0) {
					ofs << ", ";
				}
				ofs << "#" << toHexString(palette[i]);
			}
			ofs << endl << endl;
		}
	}

	void DumpTileMap(vector<Tile> tiles, ConversionOptions &options, ofstream &ofs) {
		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {
			ofs << options.BaseName << "tileset:" << endl << "DEFW ";
			for (unsigned int i = 0; i < numTiles; ++i) {
				if (i > 0) {
					ofs << ", ";
				}
				ofs << tiles[i].Name;
			}
			ofs << endl << endl;
			if (options.Palette.TransparentIndex >= 0 && !options.InterlaceMasks) {
				
				if(!options.BaseName.empty()) {
					ofs << options.BaseName << "_";
				}
				ofs << "masks_tileset:" << endl << "DEFW ";
				
				for (unsigned int i = 0; i < numTiles; ++i) {
					if (i > 0) {
						ofs << ", ";
					}
					ofs << tiles[i].Name << "_mask";
				}
				ofs << endl << endl;
			}
		}
	}

	void DumpTileMapASXXXX(vector<Tile> tiles, ConversionOptions &options, ofstream &ofs) {
		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {
			ofs << "_" << options.BaseName << "tileset::" << endl << ".dw ";
			for (unsigned int i = 0; i < numTiles; ++i) {
				if (i > 0) {
					ofs << ", ";
				}
				ofs << tiles[i].Name;
			}
			ofs << endl << endl;
			if (options.Palette.TransparentIndex >= 0 && !options.InterlaceMasks) {
				if(!options.BaseName.empty()) {
					ofs << "_" << options.BaseName;	
				}
				ofs << "_masks_tileset::" << endl << ".dw ";
				for (unsigned int i = 0; i < numTiles; ++i) {
					if (i > 0) {
						ofs << ", ";
					}
					ofs << tiles[i].Name << "_mask";
				}
				ofs << endl << endl;
			}
		}
	}

	void GenerateASM(vector<Tile> tiles, ConversionOptions &options) {
		stringstream ss;
		ss << options.OutputFileName << ".asm";
		string fileName = ss.str();

		ofstream ofs(fileName);

		ofs << "; Data created with Img2CPC - (c) Retroworks - 2007-2015" << endl;
		DumpPaletteASM(options, ofs);

		if(options.CreateTileset) {
			DumpTileMap(tiles, options, ofs);			
		}

		for (Tile t : tiles) {
			int numBytes = t.Data.size();
			if (numBytes > 0) {
				ofs << "; Tile " << t.Name << " - " << t.TileWidth << "x" << t.TileHeight << " pixels, " << t.TileWidthInBytes << "x" << t.TileHeight << " bytes." << endl;
				ofs << t.Name << ":" << endl;
				if (options.InterlaceMasks) {
					int currentByte = 0;
					for (int y = 0; y<t.TileHeight; ++y) {
						ofs << "DEFB #" << toHexString(t.MaskData[currentByte]) << ", #" << toHexString(t.Data[currentByte]);
						currentByte++;
						for (int x = 0; x<t.TileWidthInBytes - 1; ++x) {
							ofs << ", #" << toHexString(t.MaskData[currentByte]) << ", #" << toHexString(t.Data[currentByte]);
							currentByte++;
						}
						ofs << endl;
					}
				}
				else {
					int currentByte = 0;
					for (int y = 0; y<t.TileHeight; ++y) {
						ofs << "DEFB #" << toHexString(t.Data[currentByte]);
						currentByte++;
						for (int x = 0; x<t.TileWidthInBytes - 1; ++x) {
							ofs << ", #" << toHexString(t.Data[currentByte]);
							currentByte++;
						}
						ofs << endl;
					}
					if (options.Palette.TransparentIndex >= 0) {
						currentByte = 0;
						ofs << t.Name << "_MASK:" << endl;
						for (int y = 0; y<t.TileHeight; ++y) {
							ofs << "DEFB #" << toHexString(t.MaskData[currentByte]);
							currentByte++;
							for (int x = 0; x<t.TileWidthInBytes - 1; ++x) {
								ofs << ", #" << toHexString(t.MaskData[currentByte]);
								currentByte++;
							}
							ofs << endl;
						}
					}
				}
				ofs << endl;
			}
		}
		ofs.close();
	};

	void GenerateASXXXX(vector<Tile> tiles, ConversionOptions &options) {
		stringstream ss;
		ss << options.OutputFileName << ".s";
		string fileName = ss.str();

		ofstream ofs(fileName);

		ofs << "; Data created with Img2CPC - (c) Retroworks - 2007-2015" << endl;
		DumpPaletteASXXXX(options, ofs);

		if(options.CreateTileset) {
			DumpTileMapASXXXX(tiles, options, ofs);			
		}

		for (Tile t : tiles) {
			int numBytes = t.Data.size();
			if (numBytes > 0) {
				ofs << "; Tile " << t.Name << " - " << t.TileWidth << "x" << t.TileHeight << " pixels, " << t.TileWidthInBytes << "x" << t.TileHeight << " bytes." << endl;
				ofs << t.Name << "::" << endl;
				if (options.InterlaceMasks) {
					int currentByte = 0;
					for (int y = 0; y<t.TileHeight; ++y) {
						ofs << ".dw #0x" << toHexString(t.MaskData[currentByte]) << ", #0x" << toHexString(t.Data[currentByte]);
						currentByte++;
						for (int x = 0; x<t.TileWidthInBytes - 1; ++x) {
							ofs << ", #0x" << toHexString(t.MaskData[currentByte]) << ", #0x" << toHexString(t.Data[currentByte]);
							currentByte++;
						}
						ofs << endl;
					}
				}
				else {
					int currentByte = 0;
					for (int y = 0; y<t.TileHeight; ++y) {
						ofs << ".db #0x" << toHexString(t.Data[currentByte]);
						currentByte++;
						for (int x = 0; x<t.TileWidthInBytes - 1; ++x) {
							ofs << ", #0x" << toHexString(t.Data[currentByte]);
							currentByte++;
						}
						ofs << endl;
					}
					if (options.Palette.TransparentIndex >= 0) {
						currentByte = 0;
						ofs << t.Name << "_MASK::" << endl;
						for (int y = 0; y<t.TileHeight; ++y) {
							ofs << ".db #0x" << toHexString(t.MaskData[currentByte]);
							currentByte++;
							for (int x = 0; x<t.TileWidthInBytes - 1; ++x) {
								ofs << ", #0x" << toHexString(t.MaskData[currentByte]);
								currentByte++;
							}
							ofs << endl;
						}
					}
				}
				ofs << endl;
			}
		}
		ofs.close();
	};

	void GenerateBIN(vector<Tile> tiles, ConversionOptions &options) {
		stringstream ss;
		ss << options.OutputFileName << ".asm";
		string fileName = ss.str();

		string baseOutputFileName = FileUtils::RemoveExtension(options.OutputFileName);

		ofstream os(fileName);
		os << "; Data created with Img2CPC - (c) Retroworks - 2007-2015" << endl;
		
		DumpPaletteASM(options, os);
		
		if(options.CreateTileset) {
			DumpTileMap(tiles, options, os);
		}

		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {
			for (Tile t : tiles) {
				int numBytes = t.Data.size();
				if (numBytes > 0) {
					stringstream ss;
					ss << options.OutputFileName << t.Name << ".bin";
					string binFileName = ss.str();
					ofstream ofs(binFileName, ios::binary);

					os << "; Tile " << t.Name << " - " << t.TileWidth << "x" << t.TileHeight << " pixels, " << t.TileWidthInBytes << "x" << t.TileHeight << " bytes." << endl;
					os << t.Name << ":" << endl;
					os << "INCBIN \"" << FileUtils::GetFileName(binFileName) << "\"" << endl;

					if (options.InterlaceMasks) {
						for (int i = 0; i < numBytes; ++i) {
							ofs << t.MaskData[i] << t.Data[i];
						}
					}
					else {
						for (int i = 0; i < numBytes; ++i) {
							ofs << t.Data[i];
						}

						if (options.Palette.TransparentIndex >= 0) {
							stringstream ssMask;
							ssMask << options.OutputFileName << t.Name << "_mask.bin";
							string maskFileName = ssMask.str();
							ofstream ofsMask(maskFileName, ios::binary);

							os << t.Name << "_mask:" << endl;
							os << "INCBIN \"" << FileUtils::GetFileName(maskFileName) << "\"" << endl;

							for (int i = 0; i < numBytes; ++i) {
								ofsMask << t.MaskData[i];
							}
							ofsMask.close();
						}
					}
					ofs.close();
				}
			}
		}
		os.close();
		
	};

	void GenerateH(vector<Tile> tiles, ConversionOptions &options) {
		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {
			stringstream ss;
			ss << options.OutputFileName << ".h";
			string fileName = ss.str();

			ofstream ofs(fileName);
			ofs << "// Data created with Img2CPC - (c) Retroworks - 2007-2015" << endl;

			string sanitizedFileName = FileUtils::Sanitize(fileName);
			transform(sanitizedFileName.begin(), sanitizedFileName.end(), sanitizedFileName.begin(), ::toupper);

			ofs << "#ifndef _" << sanitizedFileName << "_" << endl;
			ofs << "#define _" << sanitizedFileName << "_" << endl << endl;

         // CPCtelera include required for defined types 
         ofs << "#include <types.h>" << endl << endl;         

			vector<int> palette = GetPaletteValues(options);
			unsigned int numColors = palette.size();
			if (numColors > 0) {
				ofs << "extern const u8 ";
				if(!options.BaseName.empty()) {
					ofs << options.BaseName << "_";
				}
				ofs << "palette[" << numColors << "];" << endl << endl;
			}

			if(options.CreateTileset) {
				ofs << "extern u8* const " ;
				if(!options.BaseName.empty()) {
					ofs << options.BaseName << "_";
				}
				ofs << "tileset[" << numTiles << "];" << endl << endl;
				if (!options.InterlaceMasks && options.Palette.TransparentIndex >= 0) {
					ofs << "extern u8* const "; 
					if(!options.BaseName.empty()) {
						ofs << options.BaseName << "_";						
					}
					ofs << "masks_tileset[" << numTiles << "];" << endl << endl;
				}
			}

			for (Tile t : tiles) {
				int numBytes = t.Data.size();
				if (numBytes > 0) {
					if (options.InterlaceMasks) {
						ofs << "extern const u8 " << t.Name << "[" << numBytes * 2 << "];" << endl;
					}
					else {
						ofs << "extern const u8 " << t.Name << "[" << numBytes << "];" << endl;
						if (options.Palette.TransparentIndex >= 0) {
							ofs << "extern const u8 " << t.Name << "_mask[" << numBytes << "];" << endl;
						}
					}
				}
			}
			ofs << endl << "#endif" << endl;
			ofs.close();
		}
	}

	void GenerateC(vector<Tile> tiles, ConversionOptions &options) {
		stringstream ss;
		ss << options.OutputFileName << ".c";
		string fileName = ss.str();

		ofstream ofs(fileName);
		ofs << "// Data created with Img2CPC - (c) Retroworks - 2007-2015" << endl << endl;

      // CPCtelera include required for defined types 
      ofs << "#include <types.h>" << endl << endl;

		vector<int> palette = GetPaletteValues(options);
		unsigned int numColors = palette.size();
		if (numColors > 0) {
			ofs << "// Palette uses " << ConversionOptions::ToString(options.PaletteFormat) << " values." << endl;

			ofs << "const u8 ";
			if(!options.BaseName.empty()) { 
				ofs << options.BaseName << "_";
			}
			ofs << "palette[" << numColors << "] = { ";
			for (unsigned int i = 0; i<numColors; ++i) {
				if (i > 0) {
					ofs << ", ";
				}
				ofs << "0x" << toHexString(palette[i]);
			}
			ofs << " };" << endl << endl;
		}

		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {

			if(options.CreateTileset) {
				ofs << "u8* const ";
				if(!options.BaseName.empty()) {
					ofs << options.BaseName << "_";
				}
				ofs << "tileset[" << numTiles << "] = { " << endl << "\t";
				for (unsigned int i = 0; i<numTiles; ++i) {
					if (i > 0) {
						ofs << ", ";
					}
					ofs << tiles[i].Name;
				}
				ofs << endl << "};" << endl;
				if (options.Palette.TransparentIndex >= 0 && !options.InterlaceMasks) {
					ofs << "u8* const " << options.BaseName << "_masks_tileset[" << numTiles << "] = { " << endl << "\t";
					for (unsigned int i = 0; i<numTiles; ++i) {
						if (i > 0) {
							ofs << ", ";
						}
						ofs << tiles[i].Name << "_mask";
					}
					ofs << endl << "};" << endl;
				}
			}
			
			for (Tile t : tiles) {
				int numBytes = t.Data.size();
				if (numBytes > 0) {
					ofs << "// Tile " << t.Name << ": " << t.TileWidth << "x" << t.TileHeight << " pixels, " << t.TileWidthInBytes << "x" << t.TileHeight << " bytes." << endl;
					if (options.InterlaceMasks) {
						ofs << "const u8 " << t.Name << "[" << numBytes * 2 << "] = {" << endl;
						int currentByte = 0;
						for (int y = 0; y<t.TileHeight; ++y) {
							ofs << "\t0x" << toHexString(t.MaskData[currentByte]) << ", 0x" << toHexString(t.Data[currentByte]);
							currentByte++;
							for (int x = 0; x<t.TileWidthInBytes - 1; ++x) {
								ofs << ", 0x" << toHexString(t.MaskData[currentByte]) << ", 0x" << toHexString(t.Data[currentByte]);
								currentByte++;
							}
							if (currentByte < numBytes) {
								ofs << ",";
							}
							ofs << endl;
						}
						ofs << "};" << endl;
					}
					else {
						int currentByte = 0;
						ofs << "const u8 " << t.Name << "[" << numBytes << "] = {" << endl;
						for (int y = 0; y<t.TileHeight; ++y) {
							ofs << "\t0x" << toHexString(t.Data[currentByte]);
							currentByte++;
							for (int x = 0; x<t.TileWidthInBytes - 1; ++x) {
								ofs << ", 0x" << toHexString(t.Data[currentByte]);
								currentByte++;
							}
							if (currentByte < numBytes) {
								ofs << ",";
							}
							ofs << endl;
						}
						ofs << "};" << endl;

						if (options.Palette.TransparentIndex >= 0) {
							currentByte = 0;
							ofs << "const u8 " << t.Name << "_mask[" << numBytes << "] = {" << endl;
							for (int y = 0; y<t.TileHeight; ++y) {
								ofs << "\t0x" << toHexString(t.MaskData[currentByte]);
								currentByte++;
								for (int x = 0; x<t.TileWidthInBytes - 1; ++x) {
									ofs << ", 0x" << toHexString(t.MaskData[currentByte]);
									currentByte++;
								}
								if (currentByte < numBytes) {
									ofs << ",";
								}
								ofs << endl;
							}
							ofs << "};" << endl;
						}
					}
					ofs << endl;
				}
			}
		}
		ofs.close();
	};
};

#endif