#include <stdlib.h>
#include <stdio.h>
#include "sgct.h"
#include <sstream>

sgct::Engine * gEngine;

//-----------------------
// function declarations 
//-----------------------
void myInitOGLFun();
void myPreSyncFun();
void myDrawFun();

//for syncing variables across a cluster
void myEncodeFun();
void myDecodeFun();

void drawAxes(float size);
void drawWireCube(float size);


//-----------------------
// variable declarations 
//-----------------------
sgct_utils::SGCTSphere * mySphere = NULL;
sgct_utils::SGCTSphere * mySphere1 = NULL;
sgct_utils::SGCTSphere * mySphere2 = NULL;

//store each device's transform 4x4 matrix in a shared vector
sgct::SharedVector<glm::mat4> sharedTransforms;
sgct::SharedString sharedText;
sgct::SharedObject<size_t> sharedHeadSensorIndex(0);

//pointer to a device
sgct::SGCTTrackingDevice * devicePtr = NULL;
//pointer to a tracker
sgct::SGCTTracker * trackerPtr = NULL;

//variables to share across cluster
sgct::SharedDouble curr_time(0.0);

size_t myTextureHandle;
<<<<<<< HEAD
=======
size_t myTextureHandle2;
>>>>>>> c849537344bccbba56cb722b65e722e2c3fadbda

int main( int argc, char* argv[] )
{
	// Allocate
	gEngine = new sgct::Engine( argc, argv );

	// Bind your functions
	gEngine->setInitOGLFunction( myInitOGLFun );
	gEngine->setPreSyncFunction( myPreSyncFun );
	gEngine->setDrawFunction( myDrawFun );

	// Init the engine
	if( !gEngine->init() )
	{
		delete gEngine;
		return EXIT_FAILURE;
	}

	sgct::SharedData::instance()->setEncodeFunction( myEncodeFun );
	sgct::SharedData::instance()->setDecodeFunction( myDecodeFun );

	// Main loop
	gEngine->render();

	// Clean up
	delete gEngine;

	// Exit program
	exit( EXIT_SUCCESS );
}

void myInitOGLFun()
{

  
  
  	  
	sgct::TextureManager::instance()->setAnisotropicFilterSize(8.0f);
	sgct::TextureManager::instance()->setCompression(sgct::TextureManager::S3TC_DXT);
	sgct::TextureManager::instance()->loadTexure(myTextureHandle, "sphere", "sun.jpeg", true);
<<<<<<< HEAD
	sgct::TextureManager::instance()->loadTexure(myTextureHandle, "sphere2", "earth.jpeg", true);
	sgct::TextureManager::instance()->loadTexure(myTextureHandle, "sphere3", "moon.jpeg", true);
  
	mySphere = new sgct_utils::SGCTSphere(0.2f, 30); //Jorden
	mySphere1 = new sgct_utils::SGCTSphere(0.05f, 30); //Månen
	mySphere2 = new sgct_utils::SGCTSphere(0.5f, 30); //Solen
=======
	sgct::TextureManager::instance()->loadTexure(myTextureHandle2, "sphere", "earth.jpeg", true);
  
	mySphere = new sgct_utils::SGCTSphere(0.2f, 20); //Jorden
	mySphere1 = new sgct_utils::SGCTSphere(0.05f, 20); //Månen
	mySphere2 = new sgct_utils::SGCTSphere(0.5f, 20); //Solen
>>>>>>> c849537344bccbba56cb722b65e722e2c3fadbda
		
	
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_COLOR_MATERIAL );
	glDisable( GL_LIGHTING );
	glEnable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );

	glCullFace(GL_BACK);
<<<<<<< HEAD
	glFrontFace(GL_CCW);
=======
	glFrontFace(GL_CCW); //our p
>>>>>>> c849537344bccbba56cb722b65e722e2c3fadbda
	
	//only store the tracking data on the master node
	if( gEngine->isMaster() )
	{
		size_t index = 0;
		
		//allocate shared data
		for(size_t i = 0; i < sgct::Engine::getTrackingManager()->getNumberOfTrackers(); i++)
		{
			trackerPtr = sgct::Engine::getTrackingManager()->getTrackerPtr(i);
			
			//init the shared vector with identity matrixes
			for(size_t j=0; j<trackerPtr->getNumberOfDevices(); j++)
			{
				devicePtr = trackerPtr->getDevicePtr(j);
			
				if( devicePtr->hasSensor() )
				{
					sharedTransforms.addVal( glm::mat4(1.0f) );
					
					//find the head sensor
					if( sgct::Engine::getTrackingManager()->getHeadDevicePtr() == devicePtr )
						sharedHeadSensorIndex.setVal(index);

					index++;
				}
			}
		}
	}
}

/*
	This callback is called once per render loop iteration.
*/
void myPreSyncFun()
{
	/*
	Store all transforms in the array by looping through all trackers and all devices.

	Storing values from the tracker in the pre-sync callback will guarantee
	that the values are equal for all draw calls within the same frame.
	This prevents the application from getting different tracked data for
	left and right draws using a stereoscopic display. Remember to get
	all sensor, button and analog data that will affect the rendering in this stage.
	*/

	//only store the tracking data on the master node
	if( gEngine->isMaster() )
	{
		size_t index = 0;
		std::stringstream ss;
		curr_time.setVal( sgct::Engine::getTime() );
		/*
			Loop trough all trackers (like intersense IS-900, Microsoft Kinect, PhaseSpace etc.)
		*/
		for(size_t i = 0; i < sgct::Engine::getTrackingManager()->getNumberOfTrackers(); i++)
		{
			trackerPtr = sgct::Engine::getTrackingManager()->getTrackerPtr(i);
		
			
			/*
				Loop trough all tracking devices (like headtracker, wand, stylus etc.)
			*/
			for(size_t j = 0; j < trackerPtr->getNumberOfDevices(); j++)
			{
				devicePtr = trackerPtr->getDevicePtr(j);
				
				ss << "Device " << i <<  "-" << j << ": " << devicePtr->getName() << "\n";
				
				if( devicePtr->hasSensor() )
				{
					sharedTransforms.setValAt( index, devicePtr->getWorldTransform() );
					index++;

					double trackerTime = devicePtr->getTrackerDeltaTime();
					ss << "     Sensor id: " << devicePtr->getSensorId()
						<< " freq: " << (trackerTime <= 0.0 ? 0.0 : 1.0/trackerTime) << " Hz\n";

					ss << "\n     Pos\n"
						<< "          x=" << devicePtr->getPosition().x << "\n"
						<< "          y=" << devicePtr->getPosition().y << "\n"
						<< "          z=" << devicePtr->getPosition().z << "\n";

					ss << "\n     Rot\n"
						<< "          rx=" << devicePtr->getEulerAngles().x << "\n"
						<< "          ry=" << devicePtr->getEulerAngles().y << "\n"
						<< "          rz=" << devicePtr->getEulerAngles().z << "\n";
				}

				if( devicePtr->hasButtons() )
				{
					ss << "\n     Buttons\n";
					
					for(size_t k=0; k < devicePtr->getNumberOfButtons(); k++)
					{
						ss << "          Button " << k << ": " << (devicePtr->getButton(k) ? "pressed" : "released") << "\n";
					}
				}

				if( devicePtr->hasAnalogs() )
				{
					ss << "\n     Analog axes\n";
					
					for(size_t k=0; k < devicePtr->getNumberOfAxes(); k++)
					{
						ss << "          Axis " << k << ": " << devicePtr->getAnalog(k) << "\n";
					}
				}

				ss << "\n";
			}
		}

		//store the string stream into the shared string
		sharedText.setVal( ss.str() );
	}
}

/*
	This callback can be called several times per render loop iteration.
	Using a single viewport in stereo (3D) usually results in refresh rate of 120 Hz.
*/
void myDrawFun()
{	
	double speed = 25.0;
	
	

<<<<<<< HEAD
	glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByName("sphere") );
	
	glPushMatrix();
	glRotated(curr_time.getVal() * speed, 0.0, -1.0, 0.0);
	mySphere2->draw();
	glPopMatrix();
	
=======
	//glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByName("box") );
	
	
	
	
	glPushMatrix();
		glRotated(curr_time.getVal() * speed, 0.0, -1.0, 0.0);	
			//glColor3f(1.0f,1.0f,0.0f);
			
			glActiveTexture(GL_TEXTURE0);
		
glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(myTextureHandle) );
		mySphere2->draw();
		glPopMatrix();
	
	
	glPushMatrix();
	//glTranslatef(1.0, 0.0f, 0.0f);
	glRotated(curr_time.getVal() * speed, 0.0, -1.0, 0.0);
	glTranslatef(1.0, 0.0f,0.0f);
	//glRotated(-curr_time.getVal() * speed, 0.0, -1.0, 0.0);
	//glTranslatef(-1.0, 0.0f,0.0f);
>>>>>>> c849537344bccbba56cb722b65e722e2c3fadbda
	
	
	glPushMatrix();
<<<<<<< HEAD
	glRotated(curr_time.getVal() * speed, 0.0, -1.0, 0.0);
	glTranslatef(1.0, 0.0f,0.0f);
	glRotated(curr_time.getVal() * speed*2, 0.0, -1.0, 0.0);
	glPushMatrix();	
	glRotated(curr_time.getVal() * speed*4, 0.0, -1.0, 0.0);
	glTranslatef(0.3, 0.1f, 0.0f);
	glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByName("sphere3") );
=======
	//glTranslatef(0.3, 0.1f, 0.0f);
		
	glRotated(curr_time.getVal() * speed*2, 0.0, -1.0, 0.0);
	glTranslatef(0.3, 0.1f, 0.0f);
	glRotated(curr_time.getVal() * speed*2, 0.0, -1.0, 0.0);
	//glColor3f(0.0f,1.0f,0.0f);
>>>>>>> c849537344bccbba56cb722b65e722e2c3fadbda
	mySphere1->draw();
	glPopMatrix();
	
	//glColor3f(1.0f,0.0f,0.0f);
<<<<<<< HEAD
	glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByName("sphere2") );
=======

>>>>>>> c849537344bccbba56cb722b65e722e2c3fadbda
	mySphere->draw();
	glPopMatrix();
	
	
// 	//draw some yellow cubes in space
// 	for( float i=-0.5f; i<=0.5f; i+=0.2f)
// 		for(float j=-0.5f; j<=0.5f; j+=0.2f)
// 		{
// 			glPushMatrix();
// 			glTranslatef(i, j, 0.0f);
// 			glColor3f(1.0f,1.0f,0.0f);
// 			drawWireCube(0.04f);
// 			glPopMatrix();
// 		}

	//draw a cube and axes around each wand
	for(size_t i = 0; i < sharedTransforms.getSize(); i++)
	{
		if(i != sharedHeadSensorIndex.getVal()) 
		{
			glLineWidth(2.0);

			glPushMatrix();

			glMultMatrixf( glm::value_ptr( sharedTransforms.getValAt( i ) ) );

			glColor3f(0.5f,0.5f,0.5f);
			drawWireCube(0.1f);

			drawAxes(0.1f);

			//draw pointer line
			glBegin(GL_LINES);
			glColor3f(1.0f,1.0f,0.0f);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, -5.0f);
			glEnd();

			glPopMatrix();
		}
	}

	//draw text
	float textVerticalPos = static_cast<float>(gEngine->getActiveWindowPtr()->getYResolution()) - 100.0f;
	int fontSize = 12;
	
	glColor3f(1.0f, 1.0f, 1.0f);
	sgct_text::print(sgct_text::FontManager::instance()->getFont( "SGCTFont", fontSize ),
		120.0f, textVerticalPos,
		sharedText.getVal().c_str() );
}

void myEncodeFun()
{
	sgct::SharedData::instance()->writeVector( &sharedTransforms );
	sgct::SharedData::instance()->writeString( &sharedText );
	sgct::SharedData::instance()->writeObj( &sharedHeadSensorIndex );
}

void myDecodeFun()
{
	sgct::SharedData::instance()->readVector( &sharedTransforms );
	sgct::SharedData::instance()->readString( &sharedText );
	sgct::SharedData::instance()->readObj( &sharedHeadSensorIndex );
}

void drawAxes(float size)
{
	glLineWidth(2.0);
	glBegin(GL_LINES);

	//x-axis
	glColor3f(1.0f,0.0f,0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(size, 0.0f, 0.0f);

	//y-axis
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, size, 0.0f);

	//z-axis
	glColor3f(0.0f,0.0f,1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, size);

	glEnd();
}

void drawWireCube(float size)
{
	//bottom
	glBegin(GL_LINE_STRIP);
	glVertex3f( -size, -size, -size);
	glVertex3f( size, -size, -size);
	glVertex3f( size, -size, size);
	glVertex3f( -size, -size, size);
	glVertex3f( -size, -size, -size);
	glEnd();

	//top
	glBegin(GL_LINE_STRIP);
	glVertex3f( -size, size, -size);
	glVertex3f( size, size, -size);
	glVertex3f( size, size, size);
	glVertex3f( -size, size, size);
	glVertex3f( -size, size, -size);
	glEnd();

	//sides
	glBegin(GL_LINES);
	glVertex3f( -size, -size, -size);
	glVertex3f( -size, size, -size);

	glVertex3f( size, -size, -size);
	glVertex3f( size, size, -size);

	glVertex3f( size, -size, size);
	glVertex3f( size, size, size);

	glVertex3f( -size, -size, size);
	glVertex3f( -size, size, size);
	glEnd();
}
