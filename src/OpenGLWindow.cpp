//#####################################################################
// OpenGL Window
// Copyright (c) (2018-), Bo Zhu
//#####################################################################
#include "OpenGLWindow.h"
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#ifdef USE_STB
#include "StbImage.h"
#endif
#include "File.h"
#include "OpenGLObject.h"
#include "OpenGLBufferObjects.h"
#include "OpenGLViewer.h"

using namespace OpenGLUbos;
using namespace OpenGLFbos;

OpenGLWindow* OpenGLWindow::instance=nullptr;
const unsigned int time_per_frame=20;

//////////////////////////////////////////////////////////////////////////
////OpenGLArcball
// The Arcball code is part of PhysBAM, written by Michael Lentine in 2008.
// The code distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//////////////////////////////////////////////////////////////////////////
using Transform2f=Eigen::Transform<float,2,Eigen::Affine>;
using Transform3f=Eigen::Transform<float,3,Eigen::Affine>;
using Quaternionf=Eigen::Quaternionf;
using Quaterniond=Eigen::Quaterniond;

class OpenGLArcball
{using TV=Vector<float,3>;using TV2=Vector<float,2>;
class Sphere {public:Vector3 center;float radius;Sphere():center(Vector3::Zero()),radius(1.f){}};
public:
    Sphere sphere;
    Quaternionf qNow,qDown,qDrag;
    TV2 center,vDown;
    TV vFrom,vTo,vrFrom,vrTo;
    Matrix4f mNow,mDown,mDeltaNow; //note that these only work in world space
    bool dragging;
    OpenGLWindow* window;

    OpenGLArcball():dragging(false)
    {Reinitialize();}

    void Reinitialize()
    {sphere=Sphere();
    qNow=qDown=qDrag=Quaternionf(1,0,0,0);
    center=vDown=TV2::Zero();
    vFrom=vTo=vrFrom=vrTo=TV::Zero();
	mNow=mDown=mDeltaNow=Matrix4f::Identity();

	//////for Drone test only
	//qNow=qDown=qDrag=Eigen::AngleAxisf(3.1415927f*-.05f,Vector3f::Unit(1))
	//	*Eigen::AngleAxisf(3.1415927f*.3f,Vector3f::Unit(0));
	//mNow=From_Linear(qNow.toRotationMatrix());

	mDeltaNow=From_Linear(qDrag.toRotationMatrix());
	}

    void Update(const TV2& vNow){Update_World(vNow);}

    Matrix4f From_Linear(const Matrix3f& M) // Create a homogeneous 4x4 matrix corresponding to a 3x3 transform
    {Matrix4f mat; mat<<M.data()[0],M.data()[3],M.data()[6],0,M.data()[1],M.data()[4],M.data()[7],0,M.data()[2],M.data()[5],M.data()[8],0,0,0,0,1;return mat;}

    void Update_World(const TV2 &vNow)
    {vFrom=MouseOnSphere(vDown,center,sphere.radius);vTo=MouseOnSphere(vNow,center,sphere.radius);
    if (dragging){qDrag=Qt_FromBallPoints(vFrom,vTo);qNow=qDrag*qDown;}
    Qt_ToBallPoints(qDown,vrFrom,vrTo);
    mNow=From_Linear(qNow.toRotationMatrix());
    mDeltaNow=From_Linear(qDrag.toRotationMatrix());}

    Matrix4f Value(){return mDeltaNow;}
    void Begin_Drag(const TV2 &vNow){dragging=true;vDown=vNow;Update(vNow);}
    void End_Drag(const TV2 &vNow){dragging=false;Update(vNow);qDown=qNow;mDown=mNow;}
    
private:
    TV MouseOnSphere(const TV2 &mouse,const TV2 &ballCenter,double ballRadius)
    {TV ballMouse=TV::Zero();float mag=(float)0;
    ballMouse.x()=float((mouse.x()-ballCenter.x())/ballRadius);
    ballMouse.y()=float((mouse.y()-ballCenter.y())/ballRadius);
    mag=ballMouse.squaredNorm();
    if (mag>1.0){float scale=float(1.0/sqrt(mag));ballMouse.x()*=scale;ballMouse.y()*=scale;ballMouse.z()=0.0;}
    else ballMouse.z()=float(sqrt(1-mag));
    return ballMouse;}

    Quaternionf Qt_FromBallPoints(const TV &from,const TV &to)
    {Quaternionf q;q.setFromTwoVectors(from,to);return q;}

    Vector3f Orthogonal_Vector(const Vector3f& v)
    {
        float abs_x=abs(v.x()),abs_y=abs(v.y()),abs_z=abs(v.z());
        if(abs_x<abs_y) return abs_x<abs_z?Vector3f((float)0,v.z(),-v.y()):Vector3f(v.y(),-v.x(),(float)0);
        else return abs_y<abs_z?Vector3f(-v.z(),(float)0,v.x()):Vector3f(v.y(),-v.x(),(float)0);
    }

    void Qt_ToBallPoints(const Quaternionf &q,TV &arcFrom,TV &arcTo)
    {TV vec=(q.w()>=0?(float)1:(float)-1)*q.vec().normalized();arcFrom=Orthogonal_Vector(vec).normalized();arcTo=Transform3f(q)*arcFrom;}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////OpenGLWindow
OpenGLWindow::OpenGLWindow()
{
	instance=this;

	arcball=std::make_shared<OpenGLArcball>();
	arcball->window=this;
	arcball_matrix=arcball->Value();
	rotation_matrix=arcball->Value();
}

void OpenGLWindow::Init()
{
	Initialize_Window();
	Initialize_OpenGL();
}

void OpenGLWindow::Run()
{
	glutMainLoop();
}

void OpenGLWindow::Initialize_Window()
{
	int argc=1;char* argv[1];argv[0]=(char*)(window_title.c_str());

	glutInit(&argc,argv);

#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE|GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_ALPHA);
#else
	glutInitContextVersion(4, 2);
	glutInitContextFlags(GLUT_DEBUG);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA);
#endif
    
	glutInitWindowSize(win_w,win_h);
	window_id=glutCreateWindow(window_title.c_str());

	glutSetWindow(window_id);
	glutIdleFunc(Idle_Func_Glut);
	glutTimerFunc(time_per_frame,Timer_Func_Glut,1);
	glutDisplayFunc(Display_Func_Glut);
	glutReshapeFunc(Reshape_Func_Glut);
	glutMouseFunc(Mouse_Func_Glut);
	glutMotionFunc(Motion_Func_Glut);
	glutKeyboardFunc(Keyboard_Func_Glut);
	glutKeyboardUpFunc(Keyboard_Up_Func_Glut);
	glutSpecialFunc(Keyboard_Special_Func_Glut);
	glutSpecialUpFunc(Keyboard_Special_Up_Func_Glut);
}

void OpenGLWindow::Initialize_OpenGL()
{
	if (!gladLoadGL()) {
		std::cerr << "Error: [OpenGLWindow] Cannot initialize glad" << std::endl; 
		return;
	}

	std::cout << "Opengl major version: " << GLVersion.major << ", minor version: " << GLVersion.minor << std::endl;

	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);

	glEnable(GL_MULTISAMPLE);

	Initialize_Camera();
	Initialize_Ubos();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void OpenGLWindow::Display()
{
	OpenGLShaderLibrary::Instance()->Update_Shaders();
	Update_Camera();
	Preprocess();
	Clear_Buffers();
	for(auto& obj:object_list){obj->Display();}

	Display_Text();
	if(display_offscreen)Display_Offscreen();

	GLenum gl_error=glGetError();
	if(gl_error!=GL_NO_ERROR){std::cerr<<"Error: [OpenGLWindow] "<< (const char*)gluErrorString(gl_error)<<std::endl;}
}

void OpenGLWindow::Preprocess()
{
	bool use_preprocess=false;for(auto& obj:object_list){
		OpenGLObject* o=dynamic_cast<OpenGLObject*>(obj.get());
		if(o->use_preprocess){use_preprocess=true;break;}}if(!use_preprocess)return;

	bool use_depth_fbo=false;for(auto& obj:object_list){
		OpenGLObject* o=dynamic_cast<OpenGLObject*>(obj.get());
		if(o->use_depth_fbo){use_depth_fbo=true;break;}}
	if(use_depth_fbo){
		GLuint depth_w=1024,depth_h=1024;
		auto fbo=Get_Depth_Fbo("depth");
		fbo->Resize(depth_w,depth_h);fbo->Clear();}

	for(auto& obj:object_list){
		OpenGLObject* o=dynamic_cast<OpenGLObject*>(obj.get());if(!o->use_preprocess)continue;
		o->Preprocess();}
}

void OpenGLWindow::Update_Data_To_Render()
{
	for(auto& obj:object_list){obj->Update_Data_To_Render();}
}

void OpenGLWindow::Redisplay()
{
	glutPostRedisplay();
}

void OpenGLWindow::Display_Offscreen()
{
#ifndef USE_STB
    std::cerr<<"Error: stb is required for image io"<<std::endl;
#else
    ////render to image
    if((!display_offscreen_interactive&&frame_offscreen!=frame_offscreen_rendered)||display_offscreen_interactive){
        int w=win_w;int h=win_h;
        int num_pixel=w*h;int num_comp=3;
        GLubyte* pixels=new GLubyte[num_comp*num_pixel];
        GLubyte* pixels_flipped_y=new GLubyte[num_comp*num_pixel];
        glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,pixels);
        for(int i=0;i<h;i++){int offset=w*num_comp*(h-i-1);
            std::memcpy(pixels_flipped_y+offset,pixels+w*num_comp*i,w*num_comp);}
        int wrt_frame=display_offscreen_interactive?frame_offscreen_rendered++:frame_offscreen;
        if(!File::Directory_Exists(offscreen_output_dir.c_str()))File::Create_Directory(offscreen_output_dir);
        std::stringstream ss;ss<<offscreen_output_dir<<"/"<<std::setfill('0')<<std::setw(4)<<wrt_frame<<".png";
        std::cout<<"Offscreen render to image "<<ss.str()<<std::endl;
        int rst=Stb::Write_Png(ss.str().c_str(),w,h,num_comp,pixels_flipped_y,0);
        delete pixels;delete pixels_flipped_y;
        if(!display_offscreen_interactive)frame_offscreen_rendered=frame_offscreen;}
#endif
}

void OpenGLWindow::Display_Text()
{
	if(texts.empty())return;

    // Text rendering uses features that are not available on MacOSX
    // Disable it.
#ifdef __APPLE__
    return;
#endif
    
    auto camera=Get_Camera_Ubo();
    glm::mat4& ortho=camera->object.ortho;

    /*glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(ortho));
    glColor3f(.5f,.5f,.5f);

    Vector2f step={10.f,15.f};
	int i=0;for(auto iter:texts){const std::string& text=iter.second;
		for(auto j=0;j<text.size();j++){
			Vector2f pos=Vector2f(step[0]*(float)j,(float)win_h-step[1]*((float)(i)+1.f));
			glRasterPos2f(pos[0],pos[1]);
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15,text[j]);}i++;}

    glPopAttrib();
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void OpenGLWindow::Add_Object(OpenGLObject *object)
{
    object_list.push_back(std::unique_ptr<OpenGLObject>(object));
}

void OpenGLWindow::Add_Object(std::unique_ptr<OpenGLObject>& object)
{
    object_list.push_back(std::move(object));
}

void OpenGLWindow::Clear_Objects()
{
    object_list.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void OpenGLWindow::Initialize_Camera()
{
    camera_target=Vector3f::Zero();
    camera_distance=10.f;
    Update_Clip_Planes();
}

void OpenGLWindow::Update_Camera()
{
    auto camera=Get_Camera_Ubo();
    glm::mat4& proj=camera->object.projection;
    proj=glm::perspective(glm::radians(fovy),(float)win_w/(float)win_h,nearclip,farclip);

    glm::mat4& view=camera->object.view;
    view=glm::translate(glm::mat4(),glm::vec3(0.f,0.f,(float)-camera_distance))*glm::make_mat4x4(arcball_matrix.data())*glm::make_mat4x4(rotation_matrix.data());
	view=glm::translate(view,glm::vec3(-camera_target[0],-camera_target[1],-camera_target[2]));

    glm::mat4& pvm=camera->object.pvm;

	//glm::mat4 model=glm::mat4(2.f,0.f,0.f,0.f,
	//						  0.f,1.f,0.f,0.f,
	//						  0.f,0.f,1.f,0.f,
	//						  0.f,0.f,0.f,1.f);

	//glm::mat4 model=glm::mat4(1.f);
	//model=glm::translate(model,glm::vec3(1.f,0.f,0.f));
	//for(int i=0;i<4;i++){
	//	for(int j=0;j<4;j++){
	//		std::cout<<model[i][j]<<", ";
	//	}
	//	std::cout<<std::endl;
	//}
	//std::cout<<std::endl;

    pvm=proj*view;	////assuming model matrix is identity
    glm::mat4& ortho=camera->object.ortho;
    ortho=glm::ortho(0.f,(GLfloat)win_w,0.f,(GLfloat)win_h);
    glm::vec4& position=camera->object.position;
    glm::mat4 inv_view=glm::inverse(view);
    position=glm::vec4(inv_view[3][0],inv_view[3][1],inv_view[3][2],1.f);
    camera->Set_Block_Attributes();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void OpenGLWindow::Add_Light(const Vector3f& pos,const Vector3f& Ia,const Vector3f& Id,const Vector3f& Is)
{
    auto light = Add_Point_Light(glm::vec3(pos[0],pos[1],pos[2]));
    light->amb=glm::vec4(Ia[0],Ia[1],Ia[2],1.f);
    light->dif=glm::vec4(Id[0],Id[1],Id[2],1.f);
    light->spec=glm::vec4(Is[0],Is[1],Is[2],1.f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void OpenGLWindow::Idle_Func_Glut()
{
	instance->Idle_Func();
	glutPostRedisplay();
}

void OpenGLWindow::Timer_Func_Glut(int value)
{
	instance->Timer_Func();
	glutTimerFunc(time_per_frame,Timer_Func_Glut,1);
}

void OpenGLWindow::Display_Func_Glut()
{
	instance->Display();
	glutSwapBuffers();
}

void OpenGLWindow::Reshape_Func_Glut(int w,int h)
{
	instance->Reshape_Func(w,h);
}

void OpenGLWindow::Mouse_Func_Glut(int button,int state,int x,int y)
{
	instance->Mouse_Func(button,state,x,y);
}

void OpenGLWindow::Motion_Func_Glut(int x,int y)
{
	instance->Motion_Func(x,y);
}

void OpenGLWindow::Keyboard_Func_Glut(unsigned char key,int x,int y)
{
	instance->Keyboard_Func(key,x,y);
}

void OpenGLWindow::Keyboard_Up_Func_Glut(unsigned char key,int x,int y)
{
	instance->Keyboard_Up_Func(key,x,y);
}

void OpenGLWindow::Keyboard_Special_Func_Glut(int key,int x,int y)
{
	instance->Keyboard_Special_Func(key,x,y);
}

void OpenGLWindow::Keyboard_Special_Up_Func_Glut(int key,int x,int y)
{
	instance->Keyboard_Special_Up_Func(key,x,y);
}

void OpenGLWindow::Idle_Func()
{
	if(idle_callback!=nullptr){(*idle_callback)();}
}

void OpenGLWindow::Timer_Func()
{
	if(timer_callback!=nullptr){(*timer_callback)();}
}

void OpenGLWindow::Reshape_Func(int w,int h)
{
	if (resizable) {
		win_w=w; win_h=h;
		glViewport(0, 0, (GLsizei)win_w, (GLsizei)win_h);
	}
	else {
		glutReshapeWindow(win_w, win_h);
	}
}

void OpenGLWindow::Mouse_Func(int button,int state,int x,int y)
{
	int bl_x=x;int bl_y=win_h-y;
	int left=(button==GLUT_LEFT_BUTTON?1:0)*(state==GLUT_DOWN?1:-1);
	int right=(button==GLUT_RIGHT_BUTTON?1:0)*(state==GLUT_DOWN?1:-1);
	int middle=(button==GLUT_MIDDLE_BUTTON?1:0)*(state==GLUT_DOWN?1:-1);

	bool is_focus_captured=false;
	for(auto& obj:object_list){
		is_focus_captured=obj->Mouse_Click(left,right,middle,bl_x,bl_y,Win_Width(),Win_Height());
		if(is_focus_captured){Redisplay();return;}}

	if(opengl_viewer!=nullptr){
		is_focus_captured=opengl_viewer->Mouse_Click(left,right,middle,bl_x,bl_y,Win_Width(),Win_Height());
		if(is_focus_captured){Redisplay();return;}}

    switch(button){
        case GLUT_LEFT_BUTTON:
			if(use_2d_display)break;
            {Vector2f mouse_pos=Win_To_Norm_Coord(x,y);
                if(state==GLUT_UP){
                    if(mouse_state==MouseState::Rotation){
                        arcball->End_Drag(mouse_pos);
                        arcball_matrix=arcball->Value();
                        rotation_matrix=arcball_matrix*rotation_matrix;
                        arcball_matrix=Matrix4f::Identity();}
                    mouse_state=MouseState::None;}
                else if(state==GLUT_DOWN){
                    arcball->Begin_Drag(mouse_pos);
                    mouse_state=MouseState::Rotation;}
            }break;
        case GLUT_RIGHT_BUTTON:
            {if(state==GLUT_UP){mouse_state=MouseState::None;}
                else if(state==GLUT_DOWN){mouse_state=MouseState::Zoom;}
            }break;
        case GLUT_MIDDLE_BUTTON:
            {if(state==GLUT_UP){mouse_state=MouseState::None;}
                else if(state==GLUT_DOWN){mouse_state=MouseState::Motion;Update_Mouse_Drag_Target();}
            }break;}
    mouse_x=x;mouse_y=y;
}

void OpenGLWindow::Motion_Func(int x,int y)
{
	int bl_x=x;int bl_y=win_h-y;
	bool is_focus_captured=false;
	for(auto& obj:object_list){
		is_focus_captured=obj->Mouse_Drag(bl_x,bl_y,Win_Width(),Win_Height());
		if(is_focus_captured){Redisplay();return;}}

	if(opengl_viewer!=nullptr){
		is_focus_captured=opengl_viewer->Mouse_Drag(bl_x,bl_y,Win_Width(),Win_Height());
		if(is_focus_captured){Redisplay();return;}}

    switch(mouse_state){
        case MouseState::Rotation:
        {arcball->Update(Win_To_Norm_Coord(x,y));
            arcball_matrix=arcball->Value();}break;
        case MouseState::Zoom:
        {camera_distance*=pow(1.01f,-1.f*(float)(y-mouse_y));
            Update_Clip_Planes();}break;
        case MouseState::Motion:
        {float dx=(float)(mouse_x-x);float dy=(float)(y-mouse_y);
            camera_target+=dx*target_x_drag_vector+dy*target_y_drag_vector;}break;}
    mouse_x=x;mouse_y=y;
    Redisplay();
}

void OpenGLWindow::Keyboard_Func(unsigned char key,int x,int y)
{
	int bl_x=x;int bl_y=win_h-y;
	bool is_focus_captured=false;
	for(auto& obj:object_list){
		is_focus_captured=obj->Keyboard(key,bl_x,bl_y,true);
		if(is_focus_captured)break;}
	if(is_focus_captured){Redisplay();return;}

	////Check if a keyboard callback is available
    auto c=keyboard_callbacks.find(std::string(1,key));
    if(c!=keyboard_callbacks.end()){(*c->second)();Redisplay();}
}

void OpenGLWindow::Keyboard_Up_Func(unsigned char key,int x,int y)
{
	int bl_x=x;int bl_y=win_h-y;
	bool is_focus_captured=false;
	for(auto& obj:object_list){
		is_focus_captured=obj->Keyboard(key,bl_x,bl_y,false);
		if(is_focus_captured)break;}
	if(is_focus_captured){Redisplay();return;}
}

void OpenGLWindow::Keyboard_Special_Func(int key,int x,int y)
{
	int bl_x=x;int bl_y=win_h-y;
	bool is_focus_captured=false;
	for(auto& obj:object_list){
		is_focus_captured=obj->Keyboard_Special(key,bl_x,bl_y,true);
		if(is_focus_captured)break;}
	if(is_focus_captured){Redisplay();return;}
}

void OpenGLWindow::Keyboard_Special_Up_Func(int key,int x,int y)
{
	int bl_x=x;int bl_y=win_h-y;
	bool is_focus_captured=false;
	for(auto& obj:object_list){
		is_focus_captured=obj->Keyboard_Special(key,bl_x,bl_y,false);
		if(is_focus_captured)break;}
	if(is_focus_captured){Redisplay();return;}
}

void OpenGLWindow::Quit()
{
	exit(0);
}

void OpenGLWindow::Toggle_Offscreen()
{
	display_offscreen=!display_offscreen;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f OpenGLWindow::Project(const Vector3f& pos)
{
    auto camera=Get_Camera_Ubo();
    glm::mat4 mv=camera->object.view;	////assuming model matrix is identity
    glm::mat4 proj=camera->object.projection;
    GLint viewport[4];glGetIntegerv(GL_VIEWPORT,viewport);
    glm::vec3 v(pos[0],pos[1],pos[2]);
    glm::vec4 vp;for(int i=0;i<4;i++)vp[i]=(GLfloat)viewport[i];
    glm::vec3 wpos=glm::project(v,mv,proj,vp);
    return Vector3f(wpos[0],wpos[1],wpos[2]);
}

Vector3f OpenGLWindow::Unproject(const Vector3f& win_pos)
{
    auto camera=Get_Camera_Ubo();
    glm::mat4 mv=camera->object.view;	////assuming model matrix is identity
    glm::mat4 proj=camera->object.projection;
    GLint viewport[4];glGetIntegerv(GL_VIEWPORT,viewport);
    glm::vec4 vp;for(int i=0;i<4;i++)vp[i]=(GLfloat)viewport[i];
    glm::vec3 world=glm::unProject(glm::vec3(win_pos[0],win_pos[1],win_pos[2]),mv,proj,vp);return Vector3f(world[0],world[1],world[2]);
}

GLfloat OpenGLWindow::Win_Depth(int bl_win_x,int bl_win_y)
{
	GLfloat depth;glReadPixels(bl_win_x,bl_win_y,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&depth);return depth;
}

Vector3f OpenGLWindow::Win_Coord_To_World_Coord(int bl_win_x,int bl_win_y)
{
	GLfloat depth=Win_Depth(bl_win_x,bl_win_y);
	Vector3f win_pos=Vector3f((float)bl_win_x,(float)bl_win_y,depth);
	Vector3f pos=Unproject(win_pos);
	return pos;
}

// PhysBAM Function: This function converts mouse space pixel coordinates to the normalize coordinates the arcball expects
Vector2f OpenGLWindow::Win_To_Norm_Coord(int x,int y)
{
    if(win_w>=win_h){return Vector2f(((float)x/win_w-0.5f)*2.f*((float)win_w/(float)win_h),-((float)y/(float)win_h-0.5f)*2.f);}
    else{return Vector2f(((float)x/(float)win_w-0.5f)*2.f,-((float)y/(float)win_h-0.5f)*2.f*(win_h/win_w));}
}

void OpenGLWindow::Clear_Buffers()
{
    glClearColor(.7f,.7f,.7f,0.f);	////background color
    glClearDepth(1);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void OpenGLWindow::Update_Clip_Planes()
{
    nearclip=.1f*camera_distance;
    farclip=10.f*camera_distance;
}

void OpenGLWindow::Update_Mouse_Drag_Target()
{
    Vector3f win_pos=Project(camera_target);
    Vector3f drag_target=Unproject(win_pos+Vector3f::Unit(0));
    target_x_drag_vector=drag_target-camera_target;
    drag_target=Unproject(win_pos+Vector3f::Unit(1));
    target_y_drag_vector=Vector3f(drag_target[0],drag_target[1],drag_target[2])-camera_target;
}

void OpenGLWindow::Set_Keyboard_Callback(const std::string& key,std::function<void(void)>* callback)
{
    auto c=keyboard_callbacks.find(key);
    if(c==keyboard_callbacks.end()){
        keyboard_callbacks.insert(std::pair<std::string,std::function<void(void)>* >(key,callback));}
    else{keyboard_callbacks[key]=callback;}
}

void OpenGLWindow::Set_Idle_Callback(std::function<void(void)>* callback)
{
	idle_callback=callback;
}

void OpenGLWindow::Set_Timer_Callback(std::function<void(void)>* callback)
{
	timer_callback=callback;
}

GLuint Win_Width()
{
    if(OpenGLWindow::instance==nullptr)return -1;
    else return OpenGLWindow::instance->win_w;
}

GLuint Win_Height()
{
    if(OpenGLWindow::instance==nullptr)return -1;
    else return OpenGLWindow::instance->win_h;
}

void Disable_Resize_Window()
{
	if (OpenGLWindow::instance == nullptr)return;
	else OpenGLWindow::instance->resizable=false;
}
