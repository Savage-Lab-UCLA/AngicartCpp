#ifndef PNGMINSURF_H
#define PNGMINSURF_H 1

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "lodepng.h"

#include "BinaryVolume.h"
#include "Lumens.h"
#include "utilMinSurfTests.h"

using namespace std;

struct imagePNG{
	unsigned int width, height;
	vector<unsigned char> image;
	imagePNG(unsigned int w, unsigned int h, vector<unsigned char> &i){
		width = w;
		height = h;
		image = i;
	}
};

imagePNG decodeOneStep(const char* filename);
Lumens readPNGImages(string dirName, int start, int end);
void encodeOneStep(const char* filename, std::vector<unsigned char>& image, unsigned int width, unsigned int height);

// most of the following methods are no longer useful

void writePNGLumens(const Lumens &L, string fn);
void writePNGHighlights(const Lumens &L, const vector<unsigned int> &xH, const vector<unsigned int> &yH, const vector<unsigned int> &zH, string fn);
void writePNGHighlights(const Lumens &L, const vector<unsigned int> &H, string fn);
void writePNGHighlights(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn);
void writePNGHighlightsThreeOLD(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn);
void writePNGHighlightsThree(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn,
	unsigned int splitCount = 0, unsigned int modulus = 0);
void writePNGBackbones(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn);
void writePNGBackbonesThreeOLD(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn);
void writePNGBranchingJunctions(unsigned int numBackbones, const vector<vector<unsigned int> > &branchingJunctions, string fn);
void writePNGLegend(const vector<vector<unsigned int> > &backbones, string fn);
void writePNGBackbonesThree(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn);
void writePNGBinaryVolume(const BinaryVolume &B, string fn);
void writeNiftiGzBinaryVolume(const BinaryVolume &B, string fn, const double voxdims[3]);

// below are implementations rather than just headers. usually they appear in a different .cpp file but it was organized like this when I inherited the code.
//-------------------------------------------------------

//The following method has been altered from the original example_decode.cpp of LodePNG
//Example 1
//Decode from disk to raw pixels with a single function call
imagePNG decodeOneStep(const char* filename)
{
    vector<unsigned char> image; //the raw pixels
    unsigned int width, height;
    
    //decode
    unsigned error = lodepng::decode(image, width, height, filename);
    
    //if there's an error, display it
    if(error) cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;
    
    //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
    return imagePNG(width, height, image);
}

// dirName does not need a final slash; expects image filenames of 5 padded integers
Lumens readPNGImages(string dirName, int start, int end){
    string s(dirName + "/" + paddedInt(start, 5) + ".png"); //probe for image dimensions
    imagePNG im(decodeOneStep(s.c_str())); // can move to z loop for speed
    if(im.image.size() == 0){
        cout << "\n Error loading image " << s << endl;
        return Lumens(0, 0, 0);
    }
    Lumens L(im.width, im.height, end - start + 1);
    for(int z(start); z <= end; z++){
        string s(dirName + "/" + paddedInt(z, 5) + ".png");
        imagePNG im(decodeOneStep(s.c_str()));
        for(unsigned int x(0); x < im.width; x++){
            for(unsigned int y(0); y < im.height; y++){
                int i(4*(y*im.width + x)); //pixel index in RGBA array (decodeOneStep returns a flattened vector with RGBS values for each pixel)
                L.lumens[x][y][z - start] = (double(im.image[i]) + double(im.image[i + 1]) + double(im.image[i + 2]))/3.0;
            }
        }
    }
    normalizeLumens(L);
    return L;
}

//The following method has been altered from the original example_decode.cpp of LodePNG
//Example 1
//Encode from raw pixels to disk with a single function call
//The image argument has width * height RGBA pixels or width * height * 4 bytes
void encodeOneStep(const char* filename, std::vector<unsigned char>& image, unsigned int width, unsigned int height)
{
    //Encode the image
    unsigned error = lodepng::encode(filename, image, width, height);
    
    //if there's an error, display it
    if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}

void writePNGLumens(const Lumens &L, string fn){
    vector<unsigned char> im;
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double zSum(0.0);
            for(unsigned int z(0); z < L.size[2]; z++)
                zSum += L.lumens[x][y][z];
            //int i(4*(x*L.size[0] + y));
            char v((unsigned char)floor(255*zSum/L.size[2]));
            im.push_back(v); im.push_back(v); im.push_back(v); im.push_back(255);
        }
    }
    encodeOneStep(fn.c_str(), im, L.size[0], L.size[1]);
}

void writePNGHighlights(const Lumens &L, const vector<unsigned int> &xH, const vector<unsigned int> &yH, const vector<unsigned int> &zH, string fn){
    vector<unsigned char> im;
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double zSum(0.0);
            for(unsigned int z(0); z < L.size[2]; z++)
                zSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*zSum/L.size[2]));
            im.push_back(v); im.push_back(v); im.push_back(v); im.push_back(255);
        }
    }
    for(unsigned int i(0); i < xH.size(); i++){
        int base(4*(xH[i]*L.size[0] + yH[i]));
        im[base] = 0;
        im[base + 1] = 255;
        im[base + 2] = 0;
        im[base + 3] = 255;
    }
    encodeOneStep(fn.c_str(), im, L.size[0], L.size[1]);
}

void writePNGHighlights(const Lumens &L, const vector<unsigned int> &H, string fn){
    vector<unsigned char> im;
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double zSum(0.0);
            for(unsigned int z(0); z < L.size[2]; z++)
                zSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*zSum/L.size[2]));
            im.push_back(v); im.push_back(v); im.push_back(v); im.push_back(255);
        }
    }
    for(unsigned int i(0); i < H.size(); i++){
        unsigned int xH(L.x(H[i])), yH(L.y(H[i]));
        int base(4*(xH*L.size[0] + yH));
        im[base] = 0;
        im[base + 1] = 255;
        im[base + 2] = 0;
        im[base + 3] = 255;
    }
    encodeOneStep(fn.c_str(), im, L.size[0], L.size[1]);
}

void writePNGHighlights(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn){
    vector<unsigned char> im;
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double zSum(0.0);
            for(unsigned int z(0); z < L.size[2]; z++)
                zSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*zSum/L.size[2]));
            im.push_back(v); im.push_back(v); im.push_back(v); im.push_back(255);
        }
    }
    for(unsigned int i(0); i < Hsub.size(); i++){
        unsigned int xH(L.x(Hsub[i])), yH(L.y(Hsub[i]));
        int base(4*(xH*L.size[0] + yH));
        im[base] = 0;
        im[base + 1] = 255;
        im[base + 2] = 0;
        im[base + 3] = 255;
    }
    for(unsigned int i(0); i < Hdom.size(); i++){
        unsigned int xH(L.x(Hdom[i])), yH(L.y(Hdom[i]));
        int base(4*(xH*L.size[0] + yH));
        im[base] = 0;
        im[base + 1] = 0;
        im[base + 2] = 255;
        im[base + 3] = 255;
    }
    encodeOneStep(fn.c_str(), im, L.size[0], L.size[1]);
}

void writePNGHighlightsThreeOLD(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn){
    unsigned int border(10), imwidth(L.size[0] + 3*border + L.size[1]), imheight(L.size[1] + 3*border + L.size[2]);
    vector<unsigned char> im(4*imwidth*imheight, 0);
    unsigned int rDom(0), gDom(255), bDom(0), rSub(0), gSub(0), bSub(255);
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double zSum(0.0);
            for(unsigned int z(0); z < L.size[2]; z++)
                zSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*zSum/L.size[2]));
            int base(4*((border + x)*imheight + y + border));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < Hsub.size(); i++){
        unsigned int xBB(L.x(Hsub[i])), yBB(L.y(Hsub[i]));//, zBB(L.z(Hsub[i]));
        int base(4*((border + xBB)*imheight + yBB + border));
        im[base] = rSub;
        im[base + 1] = gSub;
        im[base + 2] = bSub;
        im[base + 3] = 255;
    }
    for(unsigned int i(0); i < Hdom.size(); i++){
        unsigned int xBB(L.x(Hdom[i])), yBB(L.y(Hdom[i]));//, zBB(L.z(Hdom[i]));
        int base(4*((border + xBB)*imheight + yBB + border));
        im[base] = rDom;
        im[base + 1] = gDom;
        im[base + 2] = bDom;
        im[base + 3] = 255;
    }
    
    for(unsigned int z(0); z < L.size[2]; z++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double xSum(0.0);
            for(unsigned int x(0); x < L.size[0]; x++)
                xSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*xSum/L.size[2]));
            int base(4*((2*border + z + L.size[0])*imheight + y + border));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < Hsub.size(); i++){
        unsigned int yBB(L.y(Hsub[i])), zBB(L.z(Hsub[i]));// xBB(L.x(Hsub[i])),
        int base(4*((2*border + zBB + L.size[0])*imheight + yBB + border));
        im[base] = rSub;
        im[base + 1] = gSub;
        im[base + 2] = bSub;
        im[base + 3] = 255;
    }
    for(unsigned int i(0); i < Hdom.size(); i++){
        unsigned int yBB(L.y(Hdom[i])), zBB(L.z(Hdom[i]));//xBB(L.x(Hdom[i])),
        int base(4*((2*border + zBB + L.size[0])*imheight + yBB + border));
        im[base] = rDom;
        im[base + 1] = gDom;
        im[base + 2] = bDom;
        im[base + 3] = 255;
    }
    
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int z(0); z < L.size[2]; z++){
            double ySum(0.0);
            for(unsigned int y(0); y < L.size[1]; y++)
                ySum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*ySum/L.size[1]));
            int base(4*((border + x)*imheight + z + 2*border + L.size[1]));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < Hsub.size(); i++){
        unsigned int xBB(L.x(Hsub[i])), zBB(L.z(Hsub[i]));//, yBB(L.y(Hsub[i]))
        int base(4*((border + xBB)*imheight + zBB + 2*border + L.size[1]));
        im[base] = rSub;
        im[base + 1] = gSub;
        im[base + 2] = bSub;
        im[base + 3] = 255;
    }
    for(unsigned int i(0); i < Hdom.size(); i++){
        unsigned int xBB(L.x(Hdom[i])), zBB(L.z(Hdom[i]));//, yBB(L.y(Hdom[i]))
        int base(4*((border + xBB)*imheight + zBB + 2*border + L.size[1]));
        im[base] = rDom;
        im[base + 1] = gDom;
        im[base + 2] = bDom;
        im[base + 3] = 255;
    }
    
    encodeOneStep(fn.c_str(), im, imwidth, imheight);
}

void writePNGHighlightsThree(const Lumens &L, const vector<unsigned int> &Hsub, const vector<unsigned int> &Hdom, string fn,
                             unsigned int splitCount, unsigned int modulus){
    unsigned int border(10), imwidth(L.size[0] + 3*border + L.size[1]), imheight(L.size[1] + 3*border + L.size[2]);
    vector<unsigned char> im(4*imwidth*imheight, 0);
    unsigned char rDom(0), gDom(255), bDom(0), rSub(0), gSub(0), bSub(255);
    if(modulus > 0)
        rainbowColor(splitCount%modulus, modulus, rSub, gSub, bSub);
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double zSum(0.0);
            for(unsigned int z(0); z < L.size[2]; z++)
                zSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255.0*zSum/L.size[2]));
            int base(4*((y + border)*imwidth + border + x));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < Hsub.size(); i++){
        unsigned int xBB(L.x(Hsub[i])), yBB(L.y(Hsub[i]));//, zBB(L.z(Hsub[i]))
        int base(4*((border + yBB)*imwidth + xBB + border));
        im[base] = rSub;
        im[base + 1] = gSub;
        im[base + 2] = bSub;
        im[base + 3] = 255;
    }
    for(unsigned int i(0); i < Hdom.size(); i++){
        unsigned int xBB(L.x(Hdom[i])), yBB(L.y(Hdom[i]));//, zBB(L.z(Hdom[i]))
        int base(4*((border + yBB)*imwidth + xBB + border));
        im[base] = rDom;
        im[base + 1] = gDom;
        im[base + 2] = bDom;
        im[base + 3] = 255;
    }
    
    for(unsigned int z(0); z < L.size[2]; z++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double xSum(0.0);
            for(unsigned int x(0); x < L.size[0]; x++)
                xSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*xSum/L.size[0]));
            int base(4*((y + border)*imwidth + 2*border + z + L.size[0]));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < Hsub.size(); i++){
        unsigned int yBB(L.y(Hsub[i])), zBB(L.z(Hsub[i]));//xBB(L.x(Hsub[i])),
        int base(4*((yBB + border)*imwidth + 2*border + zBB + L.size[0]));
        im[base] = rSub;
        im[base + 1] = gSub;
        im[base + 2] = bSub;
        im[base + 3] = 255;
    }
    for(unsigned int i(0); i < Hdom.size(); i++){
        unsigned int yBB(L.y(Hdom[i])), zBB(L.z(Hdom[i]));//xBB(L.x(Hdom[i])),
        int base(4*((yBB + border)*imwidth + 2*border + zBB + L.size[0]));
        im[base] = rDom;
        im[base + 1] = gDom;
        im[base + 2] = bDom;
        im[base + 3] = 255;
    }
    
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int z(0); z < L.size[2]; z++){
            double ySum(0.0);
            for(unsigned int y(0); y < L.size[1]; y++)
                ySum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*ySum/L.size[1]));
            int base(4*((z + 2*border + L.size[1])*imwidth + border + x));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < Hsub.size(); i++){
        unsigned int xBB(L.x(Hsub[i])), zBB(L.z(Hsub[i]));// , yBB(L.y(Hsub[i]))
        int base(4*((zBB + 2*border + L.size[1])*imwidth + border + xBB));
        im[base] = rSub;
        im[base + 1] = gSub;
        im[base + 2] = bSub;
        im[base + 3] = 255;
    }
    for(unsigned int i(0); i < Hdom.size(); i++){
        unsigned int xBB(L.x(Hdom[i])), zBB(L.z(Hdom[i])); // , yBB(L.y(Hdom[i]))
        int base(4*((zBB + 2*border + L.size[1])*imwidth + border + xBB));
        im[base] = rDom;
        im[base + 1] = gDom;
        im[base + 2] = bDom;
        im[base + 3] = 255;
    }
    
    encodeOneStep(fn.c_str(), im, imwidth, imheight);
}

void writePNGBackbones(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn){
    vector<unsigned char> im;
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double zSum(0.0);
            for(unsigned int z(0); z < L.size[2]; z++)
                zSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*zSum/L.size[2]));
            im.push_back(v); im.push_back(v); im.push_back(v); im.push_back(255);
        }
    }
    for(unsigned int i(0); i < backbones.size(); i++){
        for(unsigned int j(0); j < backbones[i].size(); j++){
            unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j]));
            int base(4*(xBB*L.size[0] + yBB));
            rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
            //im[base] = 0;
            //im[base + 1] = 255;
            //im[base + 2] = 0;
            im[base + 3] = 255;
        }
    }
    encodeOneStep(fn.c_str(), im, L.size[0], L.size[1]);
}

void writePNGBackbonesThreeOLD(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn){
    unsigned int border(10), imwidth(L.size[0] + 3*border + L.size[1]), imheight(L.size[1] + 3*border + L.size[2]);
    vector<unsigned char> im(4*imwidth*imheight, 0);
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double zSum(0.0);
            for(unsigned int z(0); z < L.size[2]; z++)
                zSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*zSum/L.size[2]));
            int base(4*((border + x)*imheight + y + border));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < backbones.size(); i++){
        for(unsigned int j(0); j < backbones[i].size(); j++){
            unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
            bool highestZ(true);
            for(unsigned int m(0); m < backbones.size() && highestZ; m++){
                if(m == i)
                    continue;
                for(unsigned int n(0); n < backbones[m].size() && highestZ; n++){
                    highestZ = !(xBB == L.x(backbones[m][n])
                                 && yBB == L.y(backbones[m][n])
                                 && zBB < L.z(backbones[m][n]));
                }
            }
            if(highestZ){
                int base(4*((border + xBB)*imheight + yBB + border));
                rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
                im[base + 3] = 255;
            }
        }
    }
    
    for(unsigned int z(0); z < L.size[2]; z++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double xSum(0.0);
            for(unsigned int x(0); x < L.size[0]; x++)
                xSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*xSum/L.size[0]));
            int base(4*((2*border + L.size[0] + z)*imheight + y + border));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < backbones.size(); i++){
        for(unsigned int j(0); j < backbones[i].size(); j++){
            unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
            bool highestX(true);
            for(unsigned int m(0); m < backbones.size() && highestX; m++){
                if(m == i)
                    continue;
                for(unsigned int n(0); n < backbones[m].size() && highestX; n++){
                    highestX = !(xBB < L.x(backbones[m][n])
                                 && yBB == L.y(backbones[m][n])
                                 && zBB == L.z(backbones[m][n]));
                }
            }
            if(highestX){
                int base(4*((2*border + L.size[0] + zBB)*imheight + yBB + border));
                rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
                im[base + 3] = 255;
            }
        }
    }
    
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int z(0); z < L.size[2]; z++){
            double ySum(0.0);
            for(unsigned int y(0); y < L.size[1]; y++)
                ySum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*ySum/L.size[1]));
            int base(4*((border + x)*imheight + z + 2*border + L.size[1]));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < backbones.size(); i++){
        for(unsigned int j(0); j < backbones[i].size(); j++){
            unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
            bool highestY(true);
            for(unsigned int m(0); m < backbones.size() && highestY; m++){
                if(m == i)
                    continue;
                for(unsigned int n(0); n < backbones[m].size() && highestY; n++){
                    highestY = !(xBB == L.x(backbones[m][n])
                                 && yBB < L.y(backbones[m][n])
                                 && zBB == L.z(backbones[m][n]));
                }
            }
            if(highestY){
                int base(4*((border + xBB)*imheight + zBB + 2*border + L.size[1]));
                rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
                im[base + 3] = 255;
            }
        }
    }
    
    encodeOneStep(fn.c_str(), im, imwidth, imheight);
}

void writePNGBranchingJunctions(unsigned int numBackbones, const vector<vector<unsigned int> > &branchingJunctions, string fn){
    unsigned int imwidth(numBackbones), imheight(2*((unsigned int)branchingJunctions.size() + 1) + 1);
    vector<unsigned char> im(4*imwidth*imheight, 255);
    vector<unsigned char> r(numBackbones, 0), g(numBackbones, 0), b(numBackbones, 0);
    for(unsigned int i(0); i < numBackbones; i++){
        rainbowColor(i, numBackbones, r[i], g[i], b[i]);
        im[4*(numBackbones + i)] = r[i];
        im[4*(numBackbones + i) + 1] = g[i];
        im[4*(numBackbones + i) + 2] = b[i];
    }
    for(unsigned int i(0); i < branchingJunctions.size(); i++){
        for(unsigned int j(0); j < branchingJunctions[i].size(); j++){
            unsigned int base(4*(imwidth*(2*i + 3) + j));
            if(branchingJunctions[i][j] < numBackbones){
                im[base] = r[branchingJunctions[i][j]];
                im[base + 1] = g[branchingJunctions[i][j]];
                im[base + 2] = b[branchingJunctions[i][j]];
            }else{
                im[base] = im[base + 1] = im[base + 2] = 0;
            }
        }
    }
    encodeOneStep(fn.c_str(), im, imwidth, imheight);
}

void writePNGLegend(const vector<vector<unsigned int> > &backbones, string fn){
    vector<unsigned char> im(4*backbones.size(), 255);
    for(unsigned int i(0); i < backbones.size(); i++)
        rainbowColor(i, (unsigned int)backbones.size(), im[4*i], im[4*i + 1], im[4*i + 2]);
    encodeOneStep(fn.c_str(), im, (unsigned int)backbones.size(), 1);
}

void writePNGBackbonesThree(const Lumens &L, const vector<vector<unsigned int> > &backbones, string fn){
    unsigned int border(10), imwidth(L.size[0] + 3*border + L.size[1]), imheight(L.size[1] + 3*border + L.size[2]);
    vector<unsigned char> im(4*imwidth*imheight, 0);
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double zSum(0.0);
            for(unsigned int z(0); z < L.size[2]; z++)
                zSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*zSum/L.size[2]));
            int base(4*((border + y)*imwidth + x + border));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < backbones.size(); i++){
        for(unsigned int j(0); j < backbones[i].size(); j++){
            unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
            bool highestZ(true);
            for(unsigned int m(0); m < backbones.size() && highestZ; m++){
                if(m == i)
                    continue;
                for(unsigned int n(0); n < backbones[m].size() && highestZ; n++){
                    highestZ = !(xBB == L.x(backbones[m][n])
                                 && yBB == L.y(backbones[m][n])
                                 && zBB < L.z(backbones[m][n]));
                }
            }
            if(highestZ){
                int base(4*((border + yBB)*imwidth + xBB + border));
                rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
                im[base + 3] = 255;
            }
        }
    }
    
    for(unsigned int z(0); z < L.size[2]; z++){
        for(unsigned int y(0); y < L.size[1]; y++){
            double xSum(0.0);
            for(unsigned int x(0); x < L.size[0]; x++)
                xSum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*xSum/L.size[0]));
            int base(4*((border + y)*imwidth + 2*border + L.size[0] + z + border));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < backbones.size(); i++){
        for(unsigned int j(0); j < backbones[i].size(); j++){
            unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
            bool highestX(true);
            for(unsigned int m(0); m < backbones.size() && highestX; m++){
                if(m == i)
                    continue;
                for(unsigned int n(0); n < backbones[m].size() && highestX; n++){
                    highestX = !(xBB < L.x(backbones[m][n])
                                 && yBB == L.y(backbones[m][n])
                                 && zBB == L.z(backbones[m][n]));
                }
            }
            if(highestX){
                int base(4*((border + yBB)*imwidth + 2*border + L.size[0] + zBB + border));
                rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
                im[base + 3] = 255;
            }
        }
    }
    
    for(unsigned int x(0); x < L.size[0]; x++){
        for(unsigned int z(0); z < L.size[2]; z++){
            double ySum(0.0);
            for(unsigned int y(0); y < L.size[1]; y++)
                ySum += L.lumens[x][y][z];
            char v((unsigned char)floor(255*ySum/L.size[1]));
            int base(4*((z + 2*border + L.size[1])*imwidth + border + x));
            im[base] = im[base + 1] = im[base + 2] = v;
            im[base + 3] = 255;
        }
    }
    for(unsigned int i(0); i < backbones.size(); i++){
        for(unsigned int j(0); j < backbones[i].size(); j++){
            unsigned int xBB(L.x(backbones[i][j])), yBB(L.y(backbones[i][j])), zBB(L.z(backbones[i][j]));
            bool highestY(true);
            for(unsigned int m(0); m < backbones.size() && highestY; m++){
                if(m == i)
                    continue;
                for(unsigned int n(0); n < backbones[m].size() && highestY; n++){
                    highestY = !(xBB == L.x(backbones[m][n])
                                 && yBB < L.y(backbones[m][n])
                                 && zBB == L.z(backbones[m][n]));
                }
            }
            if(highestY){
                int base(4*((zBB + 2*border + L.size[1])*imwidth + border + xBB));
                rainbowColor(i, (unsigned int)backbones.size(), im[base], im[base + 1], im[base + 2]);
                im[base + 3] = 255;
            }
        }
    }
    
    encodeOneStep(fn.c_str(), im, imwidth, imheight);
}

void writePNGBinaryVolume(const BinaryVolume &B, string fn){
    vector<unsigned char> im(4*B.getSize(0)*B.getSize(1), 0);
    for(unsigned int x(0); x < B.getSize(0); x++){
        for(unsigned int y(0); y < B.getSize(1); y++){
            double zSum(0.0);
            for(unsigned int z(0); z < B.getSize(2); z++){
                if(B.is(x, y, z))
                    zSum += 1.0;
            }
            unsigned long long i(4*(y*B.getSize(0) + x));
            char v((unsigned char)floor(255*zSum/B.getSize(2)));
            im[i] = im[i + 1] = im[i + 2] = v;
            im[i + 3] = 255;
        }
    }
    encodeOneStep(fn.c_str(), im, (unsigned int)B.getSize(0), (unsigned int)B.getSize(1));
}

/* Writes a BinaryVolume as a gzipped NIfTI-1 (.nii.gz) uint8 mask for Python (e.g. nibabel).
 * Voxel order is standard NIfTI (x fastest). voxdims are written into pixdim / sform.
 */
void writeNiftiGzBinaryVolume(const BinaryVolume &B, string fn, const double voxdims[3]){
	const unsigned int nx((unsigned int)B.getSize(0)), ny((unsigned int)B.getSize(1)), nz((unsigned int)B.getSize(2));
	const size_t nvox((size_t)nx * (size_t)ny * (size_t)nz);
	const size_t uncompressedSize(352 + nvox); // 348-byte header + 4-byte extender + voxel data
	vector<unsigned char> raw(uncompressedSize, 0);

	auto writeLE16 = [](unsigned char *p, unsigned short v){
		p[0] = (unsigned char)(v & 0xff);
		p[1] = (unsigned char)((v >> 8) & 0xff);
	};
	auto writeLE32 = [](unsigned char *p, unsigned int v){
		p[0] = (unsigned char)(v & 0xff);
		p[1] = (unsigned char)((v >> 8) & 0xff);
		p[2] = (unsigned char)((v >> 16) & 0xff);
		p[3] = (unsigned char)((v >> 24) & 0xff);
	};
	auto writeF32 = [](unsigned char *p, float f){
		memcpy(p, &f, 4); // little-endian hosts (x86/x64/arm64 LE)
	};

	writeLE32(&raw[0], 348); // sizeof_hdr
	writeLE16(&raw[40], 3); // dim[0] = 3D
	writeLE16(&raw[42], (unsigned short)nx);
	writeLE16(&raw[44], (unsigned short)ny);
	writeLE16(&raw[46], (unsigned short)nz);
	writeLE16(&raw[70], 2); // datatype = UINT8
	writeLE16(&raw[72], 8); // bitpix
	writeF32(&raw[76], 1.0f); // pixdim[0]
	writeF32(&raw[80], (float)voxdims[0]);
	writeF32(&raw[84], (float)voxdims[1]);
	writeF32(&raw[88], (float)voxdims[2]);
	writeF32(&raw[108], 352.0f); // vox_offset
	writeF32(&raw[112], 1.0f); // scl_slope
	writeLE16(&raw[254], 1); // sform_code = NIFTI_XFORM_SCANNER_ANAT
	writeF32(&raw[280], (float)voxdims[0]); writeF32(&raw[284], 0.0f); writeF32(&raw[288], 0.0f); writeF32(&raw[292], 0.0f); // srow_x
	writeF32(&raw[296], 0.0f); writeF32(&raw[300], (float)voxdims[1]); writeF32(&raw[304], 0.0f); writeF32(&raw[308], 0.0f); // srow_y
	writeF32(&raw[312], 0.0f); writeF32(&raw[316], 0.0f); writeF32(&raw[320], (float)voxdims[2]); writeF32(&raw[324], 0.0f); // srow_z
	raw[344] = 'n'; raw[345] = '+'; raw[346] = '1'; raw[347] = '\0'; // magic for .nii

	// voxel data at offset 352 (extender bytes 348-351 already zero)
	unsigned char *data(&raw[352]);
	for(unsigned int z(0); z < nz; z++){
		for(unsigned int y(0); y < ny; y++){
			for(unsigned int x(0); x < nx; x++){
				data[(size_t)x + (size_t)y * nx + (size_t)z * nx * ny] = B.is(x, y, z) ? 1 : 0;
			}
		}
	}

	// gzip wrapper around raw deflate (lodepng provides deflate, not gzip)
	unsigned char *deflated(NULL);
	size_t deflatedSize(0);
	unsigned err(lodepng_deflate(&deflated, &deflatedSize, &raw[0], uncompressedSize, &lodepng_default_compress_settings));
	if(err || deflated == NULL){
		cout << "\n Error compressing NIfTI gzip (lodepng " << err << ")" << endl;
		free(deflated);
		return;
	}

	unsigned int crc(0xffffffffu);
	for(size_t i(0); i < uncompressedSize; i++){
		crc ^= raw[i];
		for(int k(0); k < 8; k++)
			crc = (crc >> 1) ^ (0xedb88320u & (unsigned int)(-(int)(crc & 1u)));
	}
	crc ^= 0xffffffffu;

	ofstream out(fn.c_str(), ios::binary);
	if(!out){
		cout << "\n Error opening " << fn << " for writing" << endl;
		free(deflated);
		return;
	}
	const unsigned char gzHeader[10] = {0x1f, 0x8b, 8, 0, 0, 0, 0, 0, 0, 0xff};
	out.write(reinterpret_cast<const char*>(gzHeader), 10);
	out.write(reinterpret_cast<const char*>(deflated), (streamsize)deflatedSize);
	unsigned char trailer[8];
	writeLE32(&trailer[0], crc);
	writeLE32(&trailer[4], (unsigned int)(uncompressedSize & 0xffffffffu));
	out.write(reinterpret_cast<const char*>(trailer), 8);
	out.close();
	free(deflated);
}

#endif
