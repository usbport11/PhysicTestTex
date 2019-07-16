#include "stdafx.h"
#include "classes/system/Shader.h"
#include "classes/system/Scene.h"
#include "classes/system/FPSController.h"
#include "classes/physic/PhysicWorld.h"
#include "classes/physic/objects/Hero.h"
#include "classes/physic/objects/Enemy.h"
#include "classes/physic/objects/Bullet.h"
#include "classes/image/TextureLoader.h"

bool Pause;
bool keys[1024] = {0};
int WindowWidth = 800, WindowHeight = 600;
bool EnableVsync = 1;
GLFWwindow* window;
stFPSController FPSController;

MShader Shader;
MScene Scene;
MTextureLoader TextureLoader;
stTexture* txOne;
unsigned int txOne_cnt;

MPhysicWorld PhysicWorld;
MHero* Hero = NULL;
MEnemy* Enemy = NULL;
MBullet* Bullet = NULL;
glm::vec2 HeroVelocity = glm::vec2(2.0, 2.0);
glm::vec2 BulletVelocity = glm::vec2(4.0, 4.0);

glm::vec2 MouseSceneCoord;

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void mousepos_callback(GLFWwindow* window, double x, double y) {
	MouseSceneCoord = Scene.WindowPosToWorldPos(x, y);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if(action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1) {//IMPORTANT!!! fix muse double click 
		Bullet = new MBullet;
    	Bullet->Set(glm::vec2(Hero->GetCenter().x - 10, Hero->GetCenter().y - 10), glm::vec2(20, 20), glm::vec2(0, 0), glm::vec2(1, 1), OT_ENEMY);
    	if(!PhysicWorld.AddPhysicQuad((MPhysicQuad*)Bullet)) return;
    	glm::vec2 BulletDirection = glm::normalize(MouseSceneCoord - Hero->GetCenter());
    	float BulletRotation = atan2(BulletDirection.y, BulletDirection.x);
    	Bullet->SetRotationAngle(BulletRotation);
    	Bullet->SetVelocity(b2Vec2(BulletDirection.x * BulletVelocity.x, BulletDirection.y * BulletVelocity.y));
    	PhysicWorld.FillQuadBuffer();
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}
	
	if(action == GLFW_PRESS)
    	keys[key] = true;
    else if (action == GLFW_RELEASE)
    	keys[key] = false;
}

void MoveKeysPressed() {
	b2Vec2 CurrentVelocity = b2Vec2(0.0f, 0.0f);
	if(keys['A'] || keys['D'] || keys['W'] || keys['S']) {
		if(keys['A']) {
			CurrentVelocity.x = -HeroVelocity.x;
		}
		if(keys['D']) {
			CurrentVelocity.x = HeroVelocity.x ;
		}
		if(keys['W']) {
			CurrentVelocity.y = HeroVelocity.y;
		}
		if(keys['S']) {
			CurrentVelocity.y = -HeroVelocity.y;
		}
	}
	Hero->SetVelocity(CurrentVelocity); 
}

bool InitApp() {
	LogFile<<"Starting application"<<endl;    
    glfwSetErrorCallback(error_callback);
    
    if(!glfwInit()) return false;
    window = glfwCreateWindow(WindowWidth, WindowHeight, "TestApp", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return false;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mousepos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwMakeContextCurrent(window);
    if(glfwExtensionSupported("WGL_EXT_swap_control")) {
    	LogFile<<"Window: V-Sync supported. V-Sync: "<<EnableVsync<<endl;
		glfwSwapInterval(EnableVsync);//0 - disable, 1 - enable
	}
	else LogFile<<"Window: V-Sync not supported"<<endl;
    LogFile<<"Window created: width: "<<WindowWidth<<" height: "<<WindowHeight<<endl;

	//glew
	GLenum Error = glewInit();
	if(GLEW_OK != Error) {
		LogFile<<"Window: GLEW Loader error: "<<glewGetErrorString(Error)<<endl;
		return false;
	}
	LogFile<<"GLEW initialized"<<endl;
	
	if(!CheckOpenglSupport()) return false;

	//shaders
	if(!Shader.CreateShaderProgram("shaders/main.vertexshader.glsl", "shaders/main.fragmentshader.glsl")) return false;
	if(!Shader.AddUnifrom("MVP", "MVP")) return false;
	LogFile<<"Shaders loaded"<<endl;

	//scene
	if(!Scene.Initialize(&WindowWidth, &WindowHeight)) return false;
	LogFile<<"Scene initialized"<<endl;

	//randomize
    srand(time(NULL));
    LogFile<<"Randomized"<<endl;
    
    //other initializations
    txOne = TextureLoader.LoadTexture("textures/tex04.png", 1, 1, 0, txOne_cnt, GL_NEAREST, GL_REPEAT);
    if(!txOne) return false;
    
    if(!PhysicWorld.Create(0.0f, (float)1/60, 8, 3, 0.01f, 1)) return false;
    PhysicWorld.AddWall(b2Vec2(0, 0), b2Vec2(0, 400));
    PhysicWorld.AddWall(b2Vec2(0, 0), b2Vec2(400, 0));
    PhysicWorld.AddWall(b2Vec2(400, 0), b2Vec2(400, 400));
    PhysicWorld.AddWall(b2Vec2(0, 400), b2Vec2(400, 400));
    Hero = new MHero;
    Hero->Set(glm::vec2(50, 50), glm::vec2(50, 50), glm::vec2(0, 0), glm::vec2(1, 1));
    if(!PhysicWorld.AddPhysicQuad((MPhysicQuad*)Hero)) return false;
    Enemy = new MEnemy;
    Enemy->Set(glm::vec2(300, 300), glm::vec2(50, 50), glm::vec2(0, 0), glm::vec2(1, 1));
    if(!PhysicWorld.AddPhysicQuad((MPhysicQuad*)Enemy)) return false;
    PhysicWorld.FillQuadBuffer();
    
    cout<<"Hero: "<<(int)Hero<<endl;
    cout<<"Enemy: "<<(int)Enemy<<endl;
	
	//turn off pause
	Pause = false;
    
    return true;
}

void PreRenderStep() {
	if(Pause) return;
	
	//direction move
	glm::vec2 ObjectDirection = glm::normalize(MouseSceneCoord - Hero->GetCenter());
	float ObjectRotation = atan2(ObjectDirection.y, ObjectDirection.x);
	Hero->SetRotationAngle(ObjectRotation);
	
	MoveKeysPressed();
	
	PhysicWorld.Step();
	PhysicWorld.UpdateObjects();
}

void RenderStep() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2, 0.2, 0.2, 0);
    glUseProgram(Shader.ProgramId);
	glUniformMatrix4fv(Shader.Uniforms["MVP"], 1, GL_FALSE, Scene.GetDynamicMVP());
	
	//draw functions
	PhysicWorld.DrawQuadBuffer();
}

void ClearApp() {
	//clear funstions
	PhysicWorld.Close();
	TextureLoader.DeleteTexture(txOne, txOne_cnt);
	TextureLoader.Close();
	
	memset(keys, 0, 1024);
	Shader.Close();
	LogFile<<"Application: closed"<<endl;
}

int main(int argc, char** argv) {
	LogFile<<"Application: started"<<endl;
	if(!InitApp()) {
		ClearApp();
		glfwTerminate();
		LogFile.close();
		return 0;
	}
	FPSController.Initialize(glfwGetTime());
	while(!glfwWindowShouldClose(window)) {
		FPSController.FrameStep(glfwGetTime());
    	FPSController.FrameCheck();
    	PreRenderStep();
		RenderStep();
        glfwSwapBuffers(window);
        glfwPollEvents();
	}
	ClearApp();
    glfwTerminate();
    LogFile.close();
}
