/*
 GameKit .blend file reader for Oolong Engine
 Copyright (c) 2009 Erwin Coumans http://gamekit.googlecode.com
 
 This software is provided 'as-is', without any express or implied warranty.
 In no event will the authors be held liable for any damages arising from the use of this software.
 Permission is granted to anyone to use this software for any purpose, 
 including commercial applications, and to alter it and redistribute it freely, 
 subject to the following restrictions:
 
 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
 */


#include <stdlib.h>
#include <stdio.h>

#ifdef __APPLE__

#include <OpenGLES/ES2/gl.h>

//#define	USE_IPHONE_SDK_JPEGLIB
#else
#include "GLES2/gl2.h"
#include "EGL/egl.h"

#endif//__APPLE__

#define USE_JPEG
#ifdef USE_JPEG
#define XMD_H
#ifdef  __cplusplus
extern "C" {
#endif
#include "jpeglib.h" // use included jpeglib
#ifdef  __cplusplus
}
#endif


#include <setjmp.h>

// struct for handling jpeg errors
struct my_jpeg_error_mgr
{
    // public jpeg error fields
    struct jpeg_error_mgr pub;
	
    // for longjmp, to return to caller on a fatal error
    jmp_buf setjmp_buffer;
	
};


void error_exit (j_common_ptr cinfo)
{
	// unfortunately we need to use a goto rather than throwing an exception
	// as gcc crashes under linux crashes when using throw from within
	// extern c code
	
	// Always display the message
	(*cinfo->err->output_message) (cinfo);
	
	// cinfo->err really points to a irr_error_mgr struct
	//	irr_jpeg_error_mgr *myerr = (irr_jpeg_error_mgr*) cinfo->err;
	//	longjmp(myerr->setjmp_buffer, 1);
	exit(0);
}

void output_message(j_common_ptr cinfo)
{
	// display the error message.
	//	unsigned char temp1[JMSG_LENGTH_MAX];
	//	(*cinfo->err->format_message)(cinfo, temp1);
	printf("JPEG FATAL ERROR");//,temp1, ELL_ERROR);
}

void init_source (j_decompress_ptr cinfo)
{
	// DO NOTHING
}

void skip_input_data (j_decompress_ptr cinfo, long count)
{
	jpeg_source_mgr * src = cinfo->src;
	if(count > 0)
	{
		src->bytes_in_buffer -= count;
		src->next_input_byte += count;
	}
}



void term_source (j_decompress_ptr cinfo)
{
	// DO NOTHING
}

boolean fill_input_buffer (j_decompress_ptr cinfo)
{
	// DO NOTHING
	return 1;
}
#endif //USE_JPEG



#include "autogenerated/blender.h"
#include "bMain.h"
#include "bBlenderFile.h"

#include "OolongReadBlend.h"
#include "btBulletDynamicsCommon.h"


//#define SWAP_COORDINATE_SYSTEMS
#ifdef SWAP_COORDINATE_SYSTEMS

#define IRR_X 0
#define IRR_Y 2
#define IRR_Z 1

#define IRR_X_M 1.f
#define IRR_Y_M 1.f
#define IRR_Z_M 1.f

///also winding is different
#define IRR_TRI_0_X 0
#define IRR_TRI_0_Y 2
#define IRR_TRI_0_Z 1

#define IRR_TRI_1_X 0
#define IRR_TRI_1_Y 3
#define IRR_TRI_1_Z 2
#else
#define IRR_X 0
#define IRR_Y 1
#define IRR_Z 2

#define IRR_X_M 1.f
#define IRR_Y_M 1.f
#define IRR_Z_M 1.f

///also winding is different
#define IRR_TRI_0_X 0
#define IRR_TRI_0_Y 1
#define IRR_TRI_0_Z 2

#define IRR_TRI_1_X 0
#define IRR_TRI_1_Y 2
#define IRR_TRI_1_Z 3
#endif


#define NOR_SHORTTOFLOAT 32767.0f
void norShortToFloat(const short *shnor, float *fnor)
{
	fnor[0] = shnor[0] / NOR_SHORTTOFLOAT;
	fnor[1] = shnor[1] / NOR_SHORTTOFLOAT;
	fnor[2] = shnor[2] / NOR_SHORTTOFLOAT;
}




struct BasicTexture
{
	unsigned char*	m_jpgData;
	int		m_jpgSize;
	
	int				m_width;
	int				m_height;
	GLuint			m_textureName;
	bool			m_initialized;
	GLint			m_pixelColorComponents;
	
	//contains the uncompressed R8G8B8 pixel data
	unsigned char*	m_output;

	BasicTexture(unsigned char* textureData,int width,int height)
	:m_jpgData(0),
	m_jpgSize(0),
	m_width(width),
	m_height(height),
	m_output(textureData),
	m_initialized(false)
	{
		m_pixelColorComponents = GL_RGB;
	}		
	
	BasicTexture(unsigned char* jpgData,int jpgSize)
	: m_jpgData(jpgData),
	m_jpgSize(jpgSize),
	m_output(0),
	m_textureName(-1),
	m_initialized(false)
	{
		m_pixelColorComponents = GL_RGBA;
	}
	
	virtual ~BasicTexture()
	{
		delete[] m_output;
	}
	
	//returns true if szFilename has the szExt extension
	bool checkExt(char const * szFilename, char const * szExt)
	{
		if (strlen(szFilename) > strlen(szExt))
		{
			char const * szExtension = &szFilename[strlen(szFilename) - strlen(szExt)];
			if (!strcmp(szExtension, szExt))
				return true;
		}
		return false;
	}
	
	void	loadTextureMemory(const char* fileName)
	{
		if (checkExt(fileName,".JPG") || checkExt(fileName,".jpg"))
		{
			loadJpgMemory();
		}
	}
	
	void	initOpenGLTexture()
	{
		if (m_initialized)
		{
			glBindTexture(GL_TEXTURE_2D,m_textureName);
		} else
		{
			m_initialized = true;
			

			glGenTextures(1, &m_textureName);
			glBindTexture(GL_TEXTURE_2D,m_textureName);

			//glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

//			gluBuild2DMipmaps(GL_TEXTURE_2D,3,m_width,m_height,GL_RGB,GL_UNSIGNED_BYTE,m_output);
//			glTexImage2D(GL_TEXTURE_2D,0,m_pixelColorComponents,m_width,m_height,0,m_pixelColorComponents,GL_UNSIGNED_BYTE,m_output);
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,m_width,m_height,0,GL_RGB,GL_UNSIGNED_BYTE,m_output);
			
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
//			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	//		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

		}
		
	}
	
	void	loadJpgMemory()
	{
		
#ifdef  USE_IPHONE_SDK_JPEGLIB
		NSData *imageData = [NSData dataWithBytes:m_jpgData length:m_jpgSize];
//		NSData *imageData = [NSData dataWithBytesNoCopy:m_jpgData length:m_jpgSize freeWhenDone:NO];
		UIImage *uiImage = [UIImage imageWithData:imageData];
		
		CGImageRef textureImage;
		CGContextRef textureContext;

		if( uiImage ) {
			textureImage = uiImage.CGImage;
			
			m_width = CGImageGetWidth(textureImage);
			m_height = CGImageGetHeight(textureImage);
			
						
			if(textureImage) {
				m_output = (GLubyte *) malloc(m_width * m_height * 4);
				textureContext = CGBitmapContextCreate(m_output, m_width, m_height, 8, m_width * 4, CGImageGetColorSpace(textureImage), kCGImageAlphaPremultipliedLast);
				CGContextDrawImage(textureContext, CGRectMake(0.0, 0.0, (float)m_width, (float)m_height), textureImage);
			}
			
		}			

#else
#ifdef USE_JPEG
		unsigned char **rowPtr=0;

		// allocate and initialize JPEG decompression object
		struct jpeg_decompress_struct cinfo;
		struct my_jpeg_error_mgr jerr;

		//We have to set up the error handler first, in case the initialization
		//step fails.  (Unlikely, but it could happen if you are out of memory.)
		//This routine fills in the contents of struct jerr, and returns jerr's
		//address which we place into the link field in cinfo.

		cinfo.err = jpeg_std_error(&jerr.pub);
		cinfo.err->error_exit = error_exit;
		cinfo.err->output_message = output_message;

		// compatibility fudge:
		// we need to use setjmp/longjmp for error handling as gcc-linux
		// crashes when throwing within external c code
		if (setjmp(jerr.setjmp_buffer))
		{
			// If we get here, the JPEG code has signaled an error.
			// We need to clean up the JPEG object and return.

			jpeg_destroy_decompress(&cinfo);

			
			// if the row pointer was created, we delete it.
			if (rowPtr)
				delete [] rowPtr;

			// return null pointer
			return ;
		}

		// Now we can initialize the JPEG decompression object.
		jpeg_create_decompress(&cinfo);

		// specify data source
		jpeg_source_mgr jsrc;

		// Set up data pointer
		jsrc.bytes_in_buffer = m_jpgSize;
		jsrc.next_input_byte = (JOCTET*)m_jpgData;
		cinfo.src = &jsrc;

		jsrc.init_source = init_source;
		jsrc.fill_input_buffer = fill_input_buffer;
		jsrc.skip_input_data = skip_input_data;
		jsrc.resync_to_restart = jpeg_resync_to_restart;
		jsrc.term_source = term_source;

		// Decodes JPG input from whatever source
		// Does everything AFTER jpeg_create_decompress
		// and BEFORE jpeg_destroy_decompress
		// Caller is responsible for arranging these + setting up cinfo

		// read file parameters with jpeg_read_header()
		jpeg_read_header(&cinfo, TRUE);

		cinfo.out_color_space=JCS_RGB;
		cinfo.out_color_components=3;
		cinfo.do_fancy_upsampling=FALSE;

		// Start decompressor
		jpeg_start_decompress(&cinfo);

		// Get image data
		unsigned short rowspan = cinfo.image_width * cinfo.out_color_components;
		m_width = cinfo.image_width;
		m_height = cinfo.image_height;

		// Allocate memory for buffer
		m_output = new unsigned char[rowspan * m_height];

		// Here we use the library's state variable cinfo.output_scanline as the
		// loop counter, so that we don't have to keep track ourselves.
		// Create array of row pointers for lib
		rowPtr = new unsigned char* [m_height];

		for( unsigned int i = 0; i < m_height; i++ )
			rowPtr[i] = &m_output[ i * rowspan ];

		unsigned int rowsRead = 0;

		while( cinfo.output_scanline < cinfo.output_height )
			rowsRead += jpeg_read_scanlines( &cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead );

		delete [] rowPtr;
		// Finish decompression

		jpeg_finish_decompress(&cinfo);

		// Release JPEG decompression object
		// This is an important step since it will release a good deal of memory.
		jpeg_destroy_decompress(&cinfo);
#else
	m_width  = 2;
	m_height = 2;
	m_output = new unsigned char[m_width*4 * m_height];
	for (int i=0;i<m_width*4 * m_height;i++)
	{
		m_output[i] = 255;
	}
	m_output[0] = 0;

#endif //USE_JPEG
#endif
		
	}
	
};


GfxObject::GfxObject(GLuint vboId,btCollisionObject* colObj)
:m_colObj(colObj),
m_texture(0)
{
}


void GfxObject::render(int positionLoc,int texCoordLoc, int samplerLoc, int modelMatrix)
{

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
//	glCullFace(GL_FRONT);
	
//	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc( GL_LEQUAL );
	if (m_texture)
	{

//		glEnable(GL_TEXTURE_2D);

		glActiveTexture ( GL_TEXTURE0 );

		m_texture->initOpenGLTexture();


		glBindTexture(GL_TEXTURE_2D,m_texture->m_textureName);
		
//		glDisable(GL_TEXTURE_GEN_S);
	//	glDisable(GL_TEXTURE_GEN_T);
		//glDisable(GL_TEXTURE_GEN_R);
		
	} else
	{
//		glDisable(GL_TEXTURE_2D);
	}
#ifdef USE_OPENGLES_1
	glPushMatrix();
	float m[16];
	m_colObj->getWorldTransform().getOpenGLMatrix(m);
	
	glMultMatrixf(m);
		
//	glScalef(0.1,0.1,0.1);
	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
	glTexCoordPointer(2, GL_FLOAT, sizeof(GfxVertex), &m_vertices[0].m_uv[0]);
	
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glColor4f(1, 1, 1, 1);
    glVertexPointer(3, GL_FLOAT, sizeof(GfxVertex), &m_vertices[0].m_position.getX());
 //   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	
	for (int i=0;i<m_indices.size();i++)
	{
		if (m_indices[i] > m_vertices.size())
		{
			printf("out of bounds: m_indices[%d]=%d but m_vertices.size()=%d",i,m_indices[i],m_vertices.size());
		}
	}
	glDrawElements(GL_TRIANGLES,m_indices.size(),GL_UNSIGNED_SHORT,&m_indices[0]);

	glPopMatrix();
#else
	  // Load the vertex position
   glVertexAttribPointer ( positionLoc, 3, GL_FLOAT, 
	   GL_FALSE, sizeof(GfxVertex), &m_vertices[0].m_position.getX() );
   // Load the texture coordinate
   glVertexAttribPointer ( texCoordLoc, 2, GL_FLOAT,
	   GL_FALSE, sizeof(GfxVertex), &m_vertices[0].m_uv[0] );

   glEnableVertexAttribArray ( positionLoc );
   glEnableVertexAttribArray ( texCoordLoc );

   // Bind the texture
  
//   glBindTexture ( GL_TEXTURE_2D, textureId );

   // Set the sampler texture unit to 0
   glUniform1i ( samplerLoc, 0 );


 
	static ATTRIBUTE_ALIGNED16(float mat1[16]);
	btTransform tr;
	tr.setIdentity();
	if (m_colObj)
		tr = m_colObj->getWorldTransform();
	tr.getOpenGLMatrix(mat1);

	glUniformMatrix4fv(modelMatrix,1,GL_FALSE,mat1);

	glDrawElements(GL_TRIANGLES,m_indices.size(),GL_UNSIGNED_SHORT,&m_indices[0]);
//	glLineWidth(3.0);
//	glDrawElements(GL_LINES,m_indices.size(),GL_UNSIGNED_SHORT,&m_indices[0]);

	
#endif

}
	


OolongBulletBlendReader::OolongBulletBlendReader(class btDynamicsWorld* destinationWorld)
:BulletBlendReaderNew(destinationWorld)
{
	m_notFoundTexture=0;
	m_cameraTrans.setIdentity();
}
	

OolongBulletBlendReader::~OolongBulletBlendReader()
{
	for (int i=0;i<m_graphicsObjects.size();i++)
	{
		// Delete the VBO as it is no longer needed
//		glDeleteBuffers(1, &m_graphicsObjects[i].m_ui32Vbo);
//		delete m_graphicsObjects[i].m_texture;
		
	}
}

BasicTexture* OolongBulletBlendReader::findTexture(const char* fileName)
{
	
	BasicTexture** result = m_textures.find(fileName);
	if (result)
		return *result;
	
	return 0;
}


void*	OolongBulletBlendReader::createGraphicsObject(Blender::Object* tmpObject, class btCollisionObject* bulletObject)
{
	
	GfxObject* mesh = new GfxObject(0,bulletObject);
	
	
	
	Blender::Mesh *me = (Blender::Mesh*)tmpObject->data;
	Blender::Image* image = 0;
	bParse::bMain* mainPtr = m_blendFile->getMain();
	
	if (me && me->totvert  && me->totface)
	{
		if (me->totvert> 16300)
		{
			printf("me->totvert = %d\n",me->totvert);
		}
		
		
		int maxVerts = btMin(16300,btMax(me->totface*3*2,(me->totvert-6)));
		
		GfxVertex* orgVertices= new GfxVertex[me->totvert];
		GfxVertex* newVertices= new GfxVertex[maxVerts];
		
		
		if (!me->mvert)
			return 0;
		
		float nor[3] = {0.f, 0.f, 0.f};
		
		for (int v=0;v<me->totvert;v++)
		{
			float* vt3 = &me->mvert[v].co.x;
			norShortToFloat(me->mvert[v].no, nor);
			//orgVertices[v] = btTexVert(	IRR_X_M*vt3[IRR_X],	IRR_Y_M*vt3[IRR_Y],	IRR_Z_M*vt3[IRR_Z], 	nor[IRR_X], nor[IRR_Y], nor[IRR_Z], 	irr::video::SColor(255,255,255,255), 0, 1);
			orgVertices[v].m_position.setValue(IRR_X_M*vt3[IRR_X],	IRR_Y_M*vt3[IRR_Y],	IRR_Z_M*vt3[IRR_Z]);
			orgVertices[v].m_normal[0] = nor[IRR_X];
			orgVertices[v].m_normal[1] = nor[IRR_Y];
			orgVertices[v].m_normal[2] = nor[IRR_Z];
		}
		
		
		int numVertices = 0;
		int numTriangles=0;
		int numIndices = 0;
		int currentIndex = 0;
		
		
		
		int maxNumIndices = me->totface*4*2;
		
		unsigned int* indices= new unsigned int[maxNumIndices];
		
		for (int t=0;t<me->totface;t++)
		{
			if (currentIndex>maxNumIndices)
				break;
			
			int v[4] = {me->mface[t].v1,me->mface[t].v2,me->mface[t].v3,me->mface[t].v4};
			
			bool smooth = me->mface[t].flag & 1; // ME_SMOOTH
			
			// if mface !ME_SMOOTH use face normal in place of vert norms
			if(!smooth)
			{
				btVector3 normal = (orgVertices[v[1]].m_position-orgVertices[v[0]].m_position).cross(orgVertices[v[2]].m_position-orgVertices[v[0]].m_position);
				normal.normalize();
				//normal.invert();
				
				orgVertices[v[0]].m_normal = normal;
				orgVertices[v[1]].m_normal = normal;
				orgVertices[v[2]].m_normal = normal;
				if(v[3])
					orgVertices[v[3]].m_normal = normal;
			}
			
			int originalIndex = v[IRR_TRI_0_X];
			indices[numIndices] = currentIndex;
			newVertices[indices[numIndices]] = orgVertices[originalIndex];
			if (me->mtface)
			{
				newVertices[indices[numIndices]].m_uv[0] = me->mtface[t].uv[IRR_TRI_0_X][0];
				newVertices[indices[numIndices]].m_uv[1] = 1.f - me->mtface[t].uv[IRR_TRI_0_X][1];
			} else
			{
				newVertices[indices[numIndices]].m_uv[0] = 0;
				newVertices[indices[numIndices]].m_uv[1] = 0;
			}
			numIndices++;
			currentIndex++;
			numVertices++;
			
			originalIndex = v[IRR_TRI_0_Y];
			indices[numIndices] = currentIndex;
			newVertices[indices[numIndices]] = orgVertices[originalIndex];
			if (me->mtface)
			{
				newVertices[indices[numIndices]].m_uv[0] = me->mtface[t].uv[IRR_TRI_0_Y][0];
				newVertices[indices[numIndices]].m_uv[1] = 1.f - me->mtface[t].uv[IRR_TRI_0_Y][1];
			} else
			{
				newVertices[indices[numIndices]].m_uv[0] = 0;
				newVertices[indices[numIndices]].m_uv[1] = 0;
			}
			numIndices++;
			currentIndex++;
			numVertices++;
			
			originalIndex = v[IRR_TRI_0_Z];
			indices[numIndices] = currentIndex;
			newVertices[indices[numIndices]] = orgVertices[originalIndex];
			if (me->mtface)
			{
				newVertices[indices[numIndices]].m_uv[0] = me->mtface[t].uv[IRR_TRI_0_Z][0];
				newVertices[indices[numIndices]].m_uv[1] = 1.f - me->mtface[t].uv[IRR_TRI_0_Z][1];
			} else
			{
				newVertices[indices[numIndices]].m_uv[0] = 0;
				newVertices[indices[numIndices]].m_uv[1] = 0;
			}
			numIndices++;
			currentIndex++;
			numVertices++;
			numTriangles++;
			
			if (v[3])
			{
				originalIndex = v[IRR_TRI_1_X];
				indices[numIndices]= currentIndex;
				newVertices[currentIndex] = orgVertices[originalIndex];
				if (me->mtface)
				{
					newVertices[currentIndex].m_uv[0] = me->mtface[t].uv[IRR_TRI_1_X][0];
					newVertices[currentIndex].m_uv[1] = 1.f -  me->mtface[t].uv[IRR_TRI_1_X][1];
				} else
				{
					newVertices[currentIndex].m_uv[0] = 0;
					newVertices[currentIndex].m_uv[1] = 0;
				}
				numIndices++;
				numVertices++;
				currentIndex++;
				
				originalIndex =v[IRR_TRI_1_Y];
				indices[numIndices]= currentIndex;
				newVertices[currentIndex] = orgVertices[originalIndex];
				if (me->mtface)
				{
					newVertices[currentIndex].m_uv[0] = me->mtface[t].uv[IRR_TRI_1_Y][0];
					newVertices[currentIndex].m_uv[1] = 1.f - me->mtface[t].uv[IRR_TRI_1_Y][1];
				} else
				{
					newVertices[currentIndex].m_uv[0] = 0;
					newVertices[currentIndex].m_uv[1] = 0;
				}
				numIndices++;
				numVertices++;
				currentIndex++;
				
				originalIndex = v[IRR_TRI_1_Z];
				indices[numIndices]= currentIndex;
				newVertices[currentIndex] = orgVertices[originalIndex];
				if (me->mtface)
				{
					newVertices[currentIndex].m_uv[0] = me->mtface[t].uv[IRR_TRI_1_Z][0];
					newVertices[currentIndex].m_uv[1] = 1.f - me->mtface[t].uv[IRR_TRI_1_Z][1];
				} else
				{
					newVertices[currentIndex].m_uv[0] = 0;
					newVertices[currentIndex].m_uv[1] = 0;
				}
				
				numIndices++;
				numVertices++;
				currentIndex++;
				
				numTriangles++;
			}
		}
		
		if (numTriangles>0)
		{
			mesh->m_vertices.resize(numVertices);
			int i;
			
			for (i=0;i<numVertices;i++)
			{
				mesh->m_vertices[i] = newVertices[i];
			}
			mesh->m_indices.resize(numIndices);
			for (i=0;i<numIndices;i++)
			{
				mesh->m_indices[i] = indices[i];
			}
			printf("numTriangles=%d\n",numTriangles);
			//irr::scene::ISceneNode* node = createMeshNode(newVertices,numVertices,indices,numIndices,numTriangles,bulletObject,tmpObject);
			
			//if (!meshContainer)
			//		meshContainer = new IrrlichtMeshContainer();
			
			//meshContainer->m_userPointer = tmpObject;
			//meshContainer->m_sceneNodes.push_back(node);
			
			//if (newMotionState && node)
			//	newMotionState->addIrrlichtNode(node);
			
			
		}
	}
	
	/////////////////////////////
	/// Get Texture / material
	/////////////////////////////
	
	
	if (me->mtface && me->mtface[0].tpage)
	{
		//image = (Blender::Image*)m_blendFile->getMain()->findLibPointer(me->mtface[0].tpage);
		image = me->mtface[0].tpage;
	}
	
	if (image)
	{
		const char* fileName = image->name;
		BasicTexture* texture0 = findTexture(fileName);
		
		if (!texture0)
		{
			if (image->packedfile)
			{
				void* jpgData = image->packedfile->data;
				int jpgSize = image->packedfile->size;
				if (jpgSize)
				{
					texture0 = new BasicTexture((unsigned char*)jpgData,jpgSize);
					printf("texture filename=%s\n",fileName);
					texture0->loadTextureMemory(fileName);
					
					m_textures.insert(fileName,texture0);
				}
			}
		}
		
		if (!texture0)
		{
			if (!m_notFoundTexture)
			{
				int width=256;
				int height=256;
				unsigned char*  imageData=new unsigned char[256*256*3];
				for(int y=0;y<256;++y)
				{
					const int       t=y>>4;
					unsigned char*  pi=imageData+y*256*3;
					for(int x=0;x<256;++x)
					{
						const int               s=x>>4;
						const unsigned char     b=180;
						unsigned char                   c=b+((s+t&1)&1)*(255-b);
						pi[0]= 255;
						pi[1]=pi[2]=c;pi+=3;
					}
				}
				
				m_notFoundTexture = new BasicTexture(imageData,width,height);
				
			}
			texture0 = m_notFoundTexture;
		}
		
		if (texture0 && mesh)
		{
			float scaling[3] = {tmpObject->size.x,tmpObject->size.y,tmpObject->size.z};
			mesh->m_scaling = btVector3(scaling[IRR_X],scaling[IRR_Y],scaling[IRR_Z]);
			mesh->m_texture = texture0;
			m_graphicsObjects.push_back(*mesh);
		}
		return mesh;
		
	};

	return 0;
};



void	OolongBulletBlendReader::addCamera(Blender::Object* tmpObject)
{
	float pos[3] = {tmpObject->loc.x,tmpObject->loc.y,tmpObject->loc.z};
	m_cameraTrans.setOrigin(btVector3(pos[IRR_X],pos[IRR_Y],pos[IRR_Z]));
	btMatrix3x3 mat;
	mat.setEulerZYX(tmpObject->rot.x,tmpObject->rot.y,tmpObject->rot.z);
	m_cameraTrans.setBasis(mat);
}

	
void	OolongBulletBlendReader::addLight(Blender::Object*  tmpObject)
{
	printf("added Light\n");
}

void	OolongBulletBlendReader::convertLogicBricks()
{
}

void	OolongBulletBlendReader::createParentChildHierarchy()
{
}




