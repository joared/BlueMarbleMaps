
#include <iostream>

#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/DataSets/DataSet.h"
#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Core/Feature.h"
#include "BlueMarbleMaps/Core/MapControl.h"
#include "BlueMarbleMaps/Core/Tools/ToolSet.h"
#include "BlueMarbleMaps/Core/Tools/DefaultEventHandlers.h"
#include "BlueMarbleMaps/Core/Tools/OttoTool.h"
#include "BlueMarbleMaps/Core/BlueMarbleLayout.h"
#include "Application/WindowGL.h"

#include "map_configuration.h"
#include <Keys.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <unistd.h>             // for usleep()

#ifdef __EMSCRIPTEN__
#include "libs/emscripten/emscripten_mainloop_stub.h"
#else
#include <glad/glad.h>
#endif

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES3/gl3.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

using namespace BlueMarble;

class Gui
{
private:
    MapControl* m_mapControl;
public:
    Gui()
    : m_mapControl(nullptr)
    {

    }
    ~Gui()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    void init(GLFWwindow* window, MapControl* mapControl)
    {
        m_mapControl = mapControl;
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup scaling
        ImGuiStyle& style = ImGui::GetStyle();
        float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
        style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

        // Setup Platform/Renderer backends
        #if defined(IMGUI_IMPL_OPENGL_ES2)
            // GL ES 2.0 + GLSL 100 (WebGL 1.0)
            const char* glsl_version = "#version 100";
        #elif defined(IMGUI_IMPL_OPENGL_ES3)
            // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
            const char* glsl_version = "#version 300 es";
        #elif defined(__APPLE__)
            // GL 3.2 + GLSL 150
            const char* glsl_version = "#version 150";
        #else
            // GL 3.0 + GLSL 130
            const char* glsl_version = "#version 130";
            
            //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
        #endif

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        #ifdef __EMSCRIPTEN__
            ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
        #endif
            ImGui_ImplOpenGL3_Init(glsl_version);

        // Load Fonts
        // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
        //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
        // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
        //style.FontSizeBase = 20.0f;
        //io.Fonts->AddFontDefaultVector();
        //io.Fonts->AddFontDefaultBitmap();
        //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
        //IM_ASSERT(font != nullptr);
        #ifdef __EMSCRIPTEN__
        // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
        // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
        io.IniFilename = nullptr;
        #endif
        
    }

    bool wantCaptureEvents() const
    {
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse || io.WantCaptureKeyboard;
    }

    bool update()
    {
        // Our state
        static bool show_demo_window = true;
        static bool show_another_window = false;
        static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        ImGuiIO& io = ImGui::GetIO();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            if (ImGui::ColorEdit3("clear color", (float*)&clear_color)) // Edit 3 floats representing a color
            {
                auto newColor = Color(clear_color.x*255, clear_color.y*255, clear_color.z*255);
                m_mapControl->getView()->drawable()->backgroundColor(newColor);
            }

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }


        // Rendering
        ImGui::Render();
        // int display_w, display_h;
        // glfwGetFramebufferSize(window, &display_w, &display_h);
        // glViewport(0, 0, display_w, display_h);
        // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        // glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        bool imguiBusy =
        io.WantCaptureMouse ||
        io.WantCaptureKeyboard ||
        ImGui::IsAnyItemActive() ||
        ImGui::IsAnyItemFocused();

        return imguiBusy;
    }
    
};

class GLFWMapControl : public WindowGL, public MapControl
{
public:
    GLFWMapControl()
        : m_mouseDown(false)
        , m_wireFrameMode(false)
    {
    }

    void init2()
    {
        gui.init((GLFWwindow*)getWindow(), this);
    }

    int64_t setTimer(int64_t interval) override final
    {
        // TODO, not tested
        return -1;
    }

    bool killTimer(int64_t id) override final
    {
        // TODO, not tested
        return false;
    }

    // TODO: map calls this and can be called on any thread
    void onUpdateRequest()
    {
        updateView();
        glfwPostEmptyEvent();
    }

    void keyEvent(WindowGL* window, int key, int scanCode, int action, int modifier) override
    {
        if (gui.wantCaptureEvents()) return;
        Key keyStroke(scanCode);
        std::cout << "Key is: " << keyStroke << " " << keyStroke.toString() << "\n"; 
        if (keyStroke == Key::F && action == GLFW_PRESS)
        {
            // m_wireFrameMode = !m_wireFrameMode;
            // if (m_wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            // else			  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        if (action == GLFW_PRESS)
        {
            keyDown(scanCode, getModificationKeyMask(), getGinotonicTimeStampMs());
        }
        else
        {
            keyUp(scanCode, getModificationKeyMask(), getGinotonicTimeStampMs());
        }
    }

    void resizeEvent(WindowGL* window, int width, int height) override
    {
        GLFWMapControl::resize(width, height, getGinotonicTimeStampMs());
    }

    void resizeFrameBuffer(WindowGL* window, int width, int height) override
    {
        
    }

    void getMousePos(ScreenPos& pos) const override final
    {
        double x, y;
        getMousePosition(&x, &y);
        pos.x = (int)x;
        pos.y = (int)y;
    }

    MouseButton getMouseButton() const override final
    {
        MouseButton buttons = MouseButtonNone;
        int leftButton = glfwGetMouseButton(getGLFWWindowHandle(), GLFW_MOUSE_BUTTON_LEFT);
        int rightButton = glfwGetMouseButton(getGLFWWindowHandle(), GLFW_MOUSE_BUTTON_RIGHT);
        int middleButton = glfwGetMouseButton(getGLFWWindowHandle(), GLFW_MOUSE_BUTTON_MIDDLE);
        
        if (leftButton == GLFW_PRESS)
        {
            buttons = buttons | MouseButtonLeft;
        }

        if (rightButton == GLFW_PRESS)
        {
            buttons = buttons | MouseButtonRight;
        }

        if (middleButton == GLFW_PRESS)
        {
            buttons = buttons | MouseButtonMiddle;
        }

        return buttons;
    }

    ModificationKey getModificationKeyMask() const override final
    {
        auto window = getGLFWWindowHandle();
        ModificationKey modKeys = ModificationKeyNone;
        if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        {
            modKeys = modKeys | ModificationKey::ModificationKeyCtrl;
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            modKeys = modKeys | ModificationKey::ModificationKeyShift;
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        {
            modKeys = modKeys | ModificationKey::ModificationKeyAlt;
        }

        return modKeys;
    }

    void mouseButtonEvent(WindowGL* window, int button, int action, int modifier) override
    {
        if (gui.wantCaptureEvents()) return;
        ScreenPos mousePos; getMousePos(mousePos);
        MouseButton bmmButton;

        switch (button)
        {
            case GLFW_MOUSE_BUTTON_LEFT:
                bmmButton = MouseButtonLeft;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                bmmButton = MouseButtonRight;
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                bmmButton = MouseButtonMiddle;
                break;
            default:
                std::cout << "Unknown mouse button: " << button << "\n";
                return;
        }

        switch (action)
        {
        case GLFW_PRESS:
        {
            mouseDown(bmmButton, mousePos.x, mousePos.y, getModificationKeyMask(), getGinotonicTimeStampMs());
            m_mouseDown = true;
            break;
        }
        case GLFW_RELEASE:
        {
            mouseUp(bmmButton, mousePos.x, mousePos.y, getModificationKeyMask(), getGinotonicTimeStampMs());
            m_mouseDown = false;
            break;
        }
        }
    }

    void mousePositionEvent(WindowGL* window, double x, double y) override
    {
        if (gui.wantCaptureEvents()) return;
        mouseMove(getMouseButton(), x, y, getModificationKeyMask(), getGinotonicTimeStampMs());
    }
    
    void mouseScrollEvent(WindowGL* window, double xOffs, double yOffs) override
    {
        if (gui.wantCaptureEvents()) return;
        ScreenPos mousePos; getMousePos(mousePos);
        mouseWheel(yOffs, mousePos.x, mousePos.y, getModificationKeyMask(), getGinotonicTimeStampMs());
    }

    void mouseEntered(WindowGL* window, int entered) override
    {
        
    }

    void dropEvent(WindowGL* window, int n, const char** paths) override final
    {
        if (gui.wantCaptureEvents()) return;
        std::vector<std::string> pathsVec;
        pathsVec.reserve(n);
        for (int i(0); i<n; ++i)
        {
            pathsVec.emplace_back(paths[i]);
        }

        EventManager::dropEvent(pathsVec, getGinotonicTimeStampMs());
    };

    void windowClosed(WindowGL* window) override
    {
        std::cout << "Window will close\n";
    }
    
    void* getWindow() override final
    {
        return (void*)getGLFWWindowHandle();
    }

    void loop() 
    {
        static bool updateReq = true;
        showFPS();
        if (updateReq)
        {
            pollWindowEvents();
            updateView();
            updateViewInternal();
            updateReq = gui.update();
            updateReq |= updateRequired();
            swapBuffers();
        }
        else
        {
            // gui.update();
            // swapBuffers();
            #ifndef __EMSCRIPTEN__
            waitWindowEvents();
            #else
            updateReq=true; // TODO: remove?
            #endif
            updateReq |= gui.wantCaptureEvents();
            updateReq |= updateRequired();
        }
    }

    private:
        bool m_mouseDown;
        bool m_wireFrameMode;
        Gui gui;
    
};
typedef std::shared_ptr<GLFWMapControl> GLFWMapControlPtr;

class EventObserver : public EventHandler
{
    public:
        EventObserver(const std::string& name)
            : EventHandler()
            , m_name(name)
            , m_previousEventType(EventType::Invalid)
        {}

        bool onEventFilter(const Event& event, EventHandler* target) override final
        {
            if (m_previousEventType != EventType::Invalid 
                && m_previousEventType != event.getType())
            {
                std::cout << m_name << ": " << event.toString() << "\n";
            }

            m_previousEventType = event.getType();

            if (event.getType() == EventType::DoubleClick)
            {
                return true;
            }
            return false;
        }
    private:
        std::string m_name;
        EventType   m_previousEventType;
};

int main() 
{   
    //MapControlStuff
    auto mapControl = std::make_shared<GLFWMapControl>();
    if (!mapControl->init(1000, 1000, "Hello World"))
    {
        std::cout << "Could not initiate window..." << "\n";
    }
    mapControl->init2();
    //glDebugMessageCallback(MessageCallback, 0);
    const unsigned char* version = glGetString(GL_VERSION);
    std::cout << "opengl version: " << version << "\n";

    auto view = std::make_shared<Map>();
    
    // view->crs(Crs::wgs84MercatorWeb());
    // Configure some background layers
    configureMap(view);
    mapControl->setView(view);
    view->drawable()->backgroundColor(Color(120,170,255,0));

    BMM_DEBUG() << "Setting up tools\n";
    //auto tool = std::make_shared<OttoTool>();
    auto toolSet = std::make_shared<ToolSet>();
    toolSet->addSubTool(std::make_shared<EditFeatureTool>());
    toolSet->addSubTool(std::make_shared<PointerTracerTool>());    
    toolSet->addSubTool(std::make_shared<KeyActionTool>());
    toolSet->addSubTool(std::make_shared<DebugEventHandler>());
    toolSet->addSubTool(std::make_shared<CameraControllerTwoHalfD>());
    mapControl->setTool(toolSet);

    BMM_DEBUG() << "Calling update view\n";
    // view->renderingEnabled(false); // TODO remove
    mapControl->updateView();
    
    
    BMM_DEBUG() << "Starting main loop\n";
    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    // io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!mapControl->windowShouldClose())
#endif
    
    {
        mapControl->loop();
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif
    glfwTerminate();

    return 0;
}
