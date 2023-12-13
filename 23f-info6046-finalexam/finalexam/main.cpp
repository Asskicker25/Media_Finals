#include <GL/glew.h>
#include <GL/freeglut.h>
#include "audiomanager.h"

AudioManager g_AudioManager;


void PressKey(unsigned char key, int x, int y)
{
    if (key == '1')
    {
        // Tell audio manager to apply DSP effect #1
        // TODO:
         g_AudioManager.SetActiveDSP(0);
    }

    if (key == '2')
    {
        // Tell audio manager to apply DSP effect #2
        // TODO:
        g_AudioManager.SetActiveDSP(1);
    }

    if (key == '3')
    {
        // Tell audio manager to apply DSP effect #3
        // TODO:
         g_AudioManager.SetActiveDSP(2);
    }

    if (key == '0')
    {
        // TODO:
        // Tell AudioManager to play the recorded sound
        // And use the active DSP.
        g_AudioManager.PlayRecordedSound();

    }

    if (key == '9')
    {
        // TODO:
        // Tell audio manager to play a random sound
        // And use the active DSP.
        g_AudioManager.PlayARandomSound();
    }

    if (key == ' ')
    {
        g_AudioManager.BeginRecording();
    }
}

void ReleaseKey(unsigned char key, int x, int y)
{
    if (key == ' ')
    {
        g_AudioManager.EndRecording();
    }
}

void Resize(int w, int h) { }

void Render()
{
    if (g_AudioManager.IsRecording())
    {
        glClearColor(0.5f, 1.f, 0.5f, 1.f);
    }
    else
    {
        glClearColor(0.3f, 0.3f, 1.0f, 1.f);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}

void Idle()
{
    g_AudioManager.Update();
    Render();
}


int main(int argc, char** argv)
{
    int unused1 = 0;
    char** unused2 = { };
    glutInit(&unused1, unused2);

    int width = 800;
    int height = 600;
    const char* title = "Audio Recording to a RingBuffer";

    std::vector<std::string> soundPaths;

    soundPaths.push_back("Assets/Audio/BGM.mp3");
    soundPaths.push_back("Assets/Audio/La Vi En Rose.wav");
    soundPaths.push_back("Assets/Audio/No Sunshine.wav");

    glutInitContextVersion(4, 0);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    glutInitWindowSize(width, height);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutCreateWindow(title);

    glutIdleFunc(Idle);
    glutReshapeFunc(Resize);
    glutDisplayFunc(Render);

    glutKeyboardFunc(PressKey);
    glutKeyboardUpFunc(ReleaseKey);

    glewInit();

    g_AudioManager.Initialize();

    g_AudioManager.LoadSounds(soundPaths);


    glutMainLoop();

    g_AudioManager.Destroy();

	return 0;
}