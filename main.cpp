// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"
#include "imgui_internal.h"
#include "3rdparty/imgui/backends/imgui_impl_opengl3.h"
#include "3rdparty/imgui/backends/imgui_impl_glfw.h"
#include "ImGuiFileDialog.h"
#include "CustomFont.cpp"
#include "src/proctool.h"
#include <cstdio>
#include <sstream>
#include <fstream>
#include <clocale>
#include <string>
#include <iostream>

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>  // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>  // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static bool canValidateDialog = false;

inline void InfosPane(const char* vFilter, IGFDUserDatas vUserDatas, bool* vCantContinue)  // if vCantContinue is false, the user cant validate the dialog
{
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");

    ImGui::Text("Selected Filter : %s", vFilter);

    const char* userDatas = (const char*)vUserDatas;
    if (userDatas) ImGui::Text("User Datas : %s", userDatas);

    ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);

    if (vCantContinue) *vCantContinue = canValidateDialog;
}

inline bool RadioButtonLabeled(const char* label, const char* help, bool active, bool disabled) {
    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;
    float w                 = CalcItemWidth();
    if (w == window->ItemWidthDefault) w = 0.0f;  // no push item width
    const ImGuiID id        = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, nullptr, true);
    ImVec2 bb_size          = ImVec2(style.FramePadding.x * 2 - 1, style.FramePadding.y * 2 - 1) + label_size;
    bb_size.x               = ImMax(w, bb_size.x);

    const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + bb_size);
    ItemSize(check_bb, style.FramePadding.y);

    if (!ItemAdd(check_bb, id)) return false;

    // check
    bool pressed = false;
    if (!disabled) {
        bool hovered, held;
        pressed = ButtonBehavior(check_bb, id, &hovered, &held);

        window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), style.FrameRounding);
        if (active) {
            const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, col, style.FrameRounding);
        }
    }

    // circle shadow + bg
    if (style.FrameBorderSize > 0.0f) {
        window->DrawList->AddRect(check_bb.Min + ImVec2(1, 1), check_bb.Max, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding);
        window->DrawList->AddRect(check_bb.Min, check_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
    }

    if (label_size.x > 0.0f) {
        RenderText(check_bb.GetCenter() - label_size * 0.5f, label);
    }

    if (help && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", help);

    return pressed;
}

template <typename T>
inline bool RadioButtonLabeled_BitWize(const char* vLabel, const char* vHelp, T* vContainer, T vFlag,
                                       bool vOneOrZeroAtTime     = false,  // only one selected at a time
                                       bool vAlwaysOne           = true,   // radio behavior, always one selected
                                       T vFlagsToTakeIntoAccount = (T)0, bool vDisableSelection = false,
                                       ImFont* vLabelFont = nullptr)  // radio witl use only theses flags
{
    (void)vLabelFont;  // remove unused warnings

    bool selected  = (*vContainer) & vFlag;
    const bool res = RadioButtonLabeled(vLabel, vHelp, selected, vDisableSelection);
    if (res) {
        if (!selected) {
            if (vOneOrZeroAtTime) {
                if (vFlagsToTakeIntoAccount) {
                    if (vFlag & vFlagsToTakeIntoAccount) {
                        *vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount);  // remove these flags
                        *vContainer = (T)(*vContainer | vFlag);                     // add
                    }
                } else
                    *vContainer = vFlag;  // set
            } else {
                if (vFlagsToTakeIntoAccount) {
                    if (vFlag & vFlagsToTakeIntoAccount) {
                        *vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount);  // remove these flags
                        *vContainer = (T)(*vContainer | vFlag);                     // add
                    }
                } else
                    *vContainer = (T)(*vContainer | vFlag);  // add
            }
        } else {
            if (vOneOrZeroAtTime) {
                if (!vAlwaysOne) *vContainer = (T)(0);  // remove all
            } else
                *vContainer = (T)(*vContainer & ~vFlag);  // remove one
        }
    }
    return res;
}

class CustomDrawReadOnlyCheckBoxFileDialog : public ImGuiFileDialog {
private:
    bool m_ReadOnly = false;

public:
    static CustomDrawReadOnlyCheckBoxFileDialog* Instance(CustomDrawReadOnlyCheckBoxFileDialog* vCopy = nullptr, bool vForce = false) {
        static CustomDrawReadOnlyCheckBoxFileDialog _instance;
        static CustomDrawReadOnlyCheckBoxFileDialog* _instance_copy = nullptr;
        if (vCopy || vForce) {
            _instance_copy = vCopy;
        }
        if (_instance_copy) {
            return _instance_copy;
        }
        return &_instance;
    }

public:
    void OpenDialog(const std::string& vKey, const std::string& vTitle, const char* vFilters, const std::string& vPath, const std::string& vFileName, const int& vCountSelectionMax, IGFD::UserDatas vUserDatas, ImGuiFileDialogFlags vFlags) override {
        m_ReadOnly = false;
        ImGuiFileDialog::OpenDialog(vKey, vTitle, vFilters, vPath, vFileName, vCountSelectionMax, vUserDatas, vFlags);
    }

    bool isReadOnly() const {
        return m_ReadOnly;
    }

protected:
    bool m_DrawFooter() override {
        auto& fdFile = m_FileDialogInternal.fileManager;

        float posY = ImGui::GetCursorPos().y;  // height of last bar calc
        ImGui::AlignTextToFramePadding();
        if (!fdFile.dLGDirectoryMode)
            ImGui::Text("File Name :");
        else  // directory chooser
            ImGui::Text("Directory Path :");
        ImGui::SameLine();

        // Input file fields
        float width = ImGui::GetContentRegionAvail().x;
        if (!fdFile.dLGDirectoryMode) {
            ImGuiContext& g = *GImGui;
            width -= 150.0f + g.Style.ItemSpacing.x;
        }

        ImGui::PushItemWidth(width);
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
        if (m_FileDialogInternal.dLGflags & ImGuiFileDialogFlags_ReadOnlyFileNameField) {
            flags |= ImGuiInputTextFlags_ReadOnly;
        }
        if (ImGui::InputText("##FileName", fdFile.fileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, flags)) {
            m_FileDialogInternal.isOk = true;
        }
        if (ImGui::GetItemID() == ImGui::GetActiveID()) m_FileDialogInternal.fileInputIsActive = true;
        ImGui::PopItemWidth();

        // combobox of filters
        m_FileDialogInternal.filterManager.DrawFilterComboBox(m_FileDialogInternal);

        bool res = m_DrawValidationButtons();

        ImGui::SameLine();

        if (ImGui::Checkbox("Read-Only", &m_ReadOnly)) {
            if (m_ReadOnly) {
                // remove confirm overwirte check since we are read only
                m_FileDialogInternal.dLGflags &= ~ImGuiFileDialogFlags_ConfirmOverwrite;
            } else {
                // add confirm overwirte since is what we want in our case
                m_FileDialogInternal.dLGflags |= ImGuiFileDialogFlags_ConfirmOverwrite;
            }
        }

        m_FileDialogInternal.footerHeight = ImGui::GetCursorPosY() - posY;
        return res;
    }
};

int main(int, char**) {
#ifdef _MSC_VER
    // active memory leak detector
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    auto loc = std::setlocale(LC_ALL, ".UTF8");
    if (!loc) {
        printf("setlocale fail to apply with this compiler. it seems the unicode will be NOK\n");
    }

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

        // Decide GL+GLSL versions
#if APPLE
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "image processing tools graphical interface", nullptr, nullptr);
    if (window == nullptr) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false;  // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport / Platform Windows
    io.FontAllowUserScaling = true;  // zoom wiht ctrl + mouse wheel

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiFileDialog fileDialog2, fileDialog3;

    // an override for have read only checkbox
    static bool IsFileReadOnly = false;
    CustomDrawReadOnlyCheckBoxFileDialog customFileDialog;

    ImGui::GetIO().Fonts->AddFontDefault();
    static const ImWchar icons_ranges[] = {ICON_MIN_IGFD, ICON_MAX_IGFD, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode  = true;
    icons_config.PixelSnapH = true;
    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFD, 15.0f, &icons_config, icons_ranges);

    // Our state
    bool show_demo_window = true;
    ImVec4 clear_color    = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // singleton acces
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "((Custom.+[.]h))", ImVec4(0.1f, 0.9f, 0.1f, 0.9f));  // use a regex
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC);  // add an icon for the filter type
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]");             // add an text for a filter type
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FOLDER);     // for all dirs
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "CMakeLists.txt", ImVec4(0.1f, 0.5f, 0.5f, 0.9f), ICON_IGFD_ADD);
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "doc", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_FILE_PIC);
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, nullptr, ImVec4(0.2f, 0.9f, 0.2f, 0.9f), ICON_IGFD_FILE);                              // for all link files
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FOLDER);  // for all link dirs
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FILE);   // for all link files
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.5f, 0.8f, 0.5f, 0.9f), ICON_IGFD_SAVE);
    // set file style with a lambda function
    // return true is a file style was defined
    ImGuiFileDialog::Instance()->SetFileStyle([](const IGFD::FileInfos& vFileInfos, IGFD::FileStyle& vOutStyle) -> bool {
        if (!vFileInfos.fileNameExt.empty() && vFileInfos.fileNameExt[0] == '.') {
            vOutStyle.color = ImVec4(0.0f, 0.9f, 0.9f, 1.0f);
            vOutStyle.icon = ICON_IGFD_REMOVE;
            return true;
        }
        return false;
    });

    // just for show multi dialog instance behavior (here use for show directory query dialog)
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".h", ImVec4(0.0f, 1.0f, 0.0f, 0.9f));
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC);  // add an icon for the filter type
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]");             // add an text for a filter type
    fileDialog2.SetFileStyle(IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);

    fileDialog3.SetFileStyle(IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
    fileDialog3.SetFileStyle(IGFD_FileStyleByExtention, ".h", ImVec4(0.0f, 1.0f, 0.0f, 0.9f));
    fileDialog3.SetFileStyle(IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
    fileDialog3.SetFileStyle(IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
    fileDialog3.SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC);  // add an icon for the filter type
    fileDialog3.SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]");             // add an text for a filter type
    fileDialog3.SetFileStyle(IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);
    fileDialog3.SetFileStyle(IGFD_FileStyleByFullName, "doc", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_FILE_PIC);

    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "((Custom.+[.]h))", ImVec4(0.1f, 0.9f, 0.1f, 0.9f));  // use a regex
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC);  // add an icon for the filter type
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]");             // add an text for a filter type
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FOLDER);     // for all dirs
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "CMakeLists.txt", ImVec4(0.1f, 0.5f, 0.5f, 0.9f), ICON_IGFD_ADD);
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "doc", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_FILE_PIC);
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, nullptr, ImVec4(0.2f, 0.9f, 0.2f, 0.9f), ICON_IGFD_FILE);                              // for all link files
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FOLDER);  // for all link dirs
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FILE);   // for all link files
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.5f, 0.8f, 0.5f, 0.9f), ICON_IGFD_SAVE);
    // set file style with a lambda function
    // return true is a file style was defined
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle([](const IGFD::FileInfos& vFile, IGFD::FileStyle& vOutStyle) -> bool {
        if (!vFile.fileNameExt.empty() && vFile.fileNameExt[0] == '.') {
            vOutStyle.color = ImVec4(0.0f, 0.9f, 0.9f, 1.0f);
            vOutStyle.icon = ICON_IGFD_REMOVE;
            return true;
        }
        return false;
    });

#ifdef USE_BOOKMARK
    // load bookmarks
    std::ifstream docFile_1("bookmarks_1.conf", std::ios::in);
    if (docFile_1.is_open()) {
        std::stringstream strStream;
        strStream << docFile_1.rdbuf();  // read the file
        ImGuiFileDialog::Instance()->DeserializeBookmarks(strStream.str());
        docFile_1.close();
    }

    std::ifstream docFile_2("bookmarks_2.conf", std::ios::in);
    if (docFile_2.is_open()) {
        std::stringstream strStream;
        strStream << docFile_2.rdbuf();  // read the file
        fileDialog2.DeserializeBookmarks(strStream.str());
        docFile_2.close();
    }

    // add bookmark by code (why ? because we can :-) )
    ImGuiFileDialog::Instance()->AddBookmark("Current Dir", ".");
#endif

    static std::string filePathName;
    static std::string filePath;
    static std::string filter;
    static std::string userDatas;
    static std::string input_directory, output_directory;
    static std::vector<std::pair<std::string, std::string>> selection = {};

    static bool UseWindowContraints  = true;
    static ImGuiFileDialogFlags flags = ImGuiFileDialogFlags_Default;
    static IGFD_ResultMode resultMode = IGFD_ResultMode_AddIfNoFileExt;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        ImGui::Begin("Main Entrance");  // Create a window called "Hello, world!" and append into it.
        {
            ImGui::Text("Main Entrance %s : ", IMGUIFILEDIALOG_VERSION);
            ImGui::Separator();

            ImGui::ColorEdit3("clear color", (float*)&clear_color);  // Edit 3 floats representing a color
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Checkbox("Demo Window", &show_demo_window);  // Edit bools storing our window open/close state
            if (ImGui::CollapsingHeader("Settings")) {
#ifdef USE_EXPLORATION_BY_KEYS
                static float flashingAttenuationInSeconds = 1.0f;
                if (ImGui::Button("R##resetflashlifetime")) {
                    flashingAttenuationInSeconds = 1.0f;
                    ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
                    fileDialog2.SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);

                }
                ImGui::SameLine();
                ImGui::PushItemWidth(200);
                if (ImGui::SliderFloat("Flash lifetime (s)", &flashingAttenuationInSeconds, 0.01f, 5.0f)) {
                    ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
                    fileDialog2.SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);

                }
                ImGui::PopItemWidth();
#endif
                ImGui::Separator();
                ImGui::Checkbox("Use file dialog constraint", &UseWindowContraints);
                ImGui::Text("Constraints is used here for define min/max file dialog size");
                ImGui::Separator();

                ImGui::Text("ImGuiFileDialog Flags : ");
                ImGui::Indent();
                {
                    ImGui::Text("Commons :");
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Modal", "Open dialog in modal mode", &flags, ImGuiFileDialogFlags_Modal);
                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Overwrite", "Overwrite verification before dialog closing", &flags, ImGuiFileDialogFlags_ConfirmOverwrite);
                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Hidden Files", "Hide Hidden Files", &flags, ImGuiFileDialogFlags_DontShowHiddenFiles);

                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Disable Directory Creation", "Disable Directory Creation button in dialog", &flags, ImGuiFileDialogFlags_DisableCreateDirectoryButton);
#ifdef USE_THUMBNAILS
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Disable thumbnails mode", "Disable thumbnails display in dialo", &flags, ImGuiFileDialogFlags_DisableThumbnailMode);
#endif  // USE_THUMBNAILS
#ifdef USE_BOOKMARK
                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Disable bookmark mode", "Disable bookmark display in dialo", &flags, ImGuiFileDialogFlags_DisableBookmarkMode);
#endif  // USE_BOOKMARK

                    ImGui::Text("Hide Column by default : (saved in imgui.ini, \n\tso defined when the imgui.ini is not existing)");
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Column Type", "Hide Column file type by default", &flags, ImGuiFileDialogFlags_HideColumnType);
                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Column Size", "Hide Column file Size by default", &flags, ImGuiFileDialogFlags_HideColumnSize);
                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Column Date", "Hide Column file Date by default", &flags, ImGuiFileDialogFlags_HideColumnDate);

                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Case Insensitive Extentions", "will not take into account the case of file extentions", &flags, ImGuiFileDialogFlags_CaseInsensitiveExtention);

                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Disable quick path selection", "Disable the quick path selection", &flags, ImGuiFileDialogFlags_DisableQuickPathSelection);
                    
                    ImGui::Separator();
                    ImGui::Text("Result Modes : for GetFilePathName and GetSelection");
                    
                    if (RadioButtonLabeled("Add If No File Ext", nullptr, resultMode == IGFD_ResultMode_::IGFD_ResultMode_AddIfNoFileExt, false)) {
                        resultMode = IGFD_ResultMode_::IGFD_ResultMode_AddIfNoFileExt;
                    }
                    ImGui::SameLine();
                    if (RadioButtonLabeled("Overwrite File Ext", nullptr, resultMode == IGFD_ResultMode_::IGFD_ResultMode_OverwriteFileExt, false)) {
                        resultMode = IGFD_ResultMode_::IGFD_ResultMode_OverwriteFileExt;
                    }
                    ImGui::SameLine();
                    if (RadioButtonLabeled("Keep Input File", nullptr, resultMode == IGFD_ResultMode_::IGFD_ResultMode_KeepInputFile, false)) {
                        resultMode = IGFD_ResultMode_::IGFD_ResultMode_KeepInputFile;
                    }
                }
                ImGui::Unindent();
            }

            /// for choosing directories
            if (ImGui::CollapsingHeader("Choose Directories :")) {
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Input Directory: ")) {
                    // let filters be null for open directory chooser
                    fileDialog2.OpenDialog("ChooseDirDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a Directory", nullptr, ".", 1, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Output Directory: ")) {
                    // let filters be null for open directory chooser
                    fileDialog3.OpenDialog("ChooseDirDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a Directory", nullptr, ".", 1, nullptr, flags);
                }
            }

            /// for selecting functionality
            if (ImGui::CollapsingHeader("img2yuv :")) {
                const char* items[] = {"L", "R"};
                static int item_current = 0; // This will be 0 for "L", 1 for "R"
                ImGui::Combo("Choose the position of image", &item_current, items, IM_ARRAYSIZE(items));

                static char left_marker[64] = "left";
                static char right_marker[64] = "right";
                static ImGuiInputFlags flags_img2yuv = ImGuiInputTextFlags_AlwaysOverwrite;

                static bool is_button_clicked = false;
                static bool is_animate = false;

                if (ImGui::TreeNode("Left Marker : ")) {
                    if (item_current == 0) { // Only show this if "L" is selected
                        ImGui::InputText("##sourceLeft", left_marker, IM_ARRAYSIZE(left_marker), flags_img2yuv);
                    }
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Right Marker : ")) {
                    if (item_current == 1) { // Only show this if "R" is selected
                        ImGui::InputText("##sourceRight", right_marker, IM_ARRAYSIZE(right_marker), flags_img2yuv);
                    }
                    ImGui::TreePop();
                }

                if (ImGui::Button("Run")) {
                    // Use the text from the selected input box
                    is_button_clicked = true;
                    std::cout << "Left Marker: " << left_marker << std::endl;
                    std::cout << "Right Marker: " << right_marker << std::endl;
                    std::cout << "image position: " << items[item_current] << std::endl;
                    std::cout << "input directory: " << input_directory << std::endl;
                    std::cout << "output directory: " << output_directory << std::endl;
                    nvpimgproc::handle::Handler handler(input_directory, output_directory);
                    handler.process_img2yuv((std::string&)items[item_current], reinterpret_cast<std::string&>(left_marker), reinterpret_cast<std::string&>(right_marker));
                }

//                if(is_button_clicked) {
//                    is_animate = true;
//                    static double refresh_time = 0.0;
//                    if (!is_animate || refresh_time == 0.0) {
//                        refresh_time = ImGui::GetTime();
//                    }
//                }
            }


            if (ImGui::CollapsingHeader("ImGuiFileDialog Return's :")) {
                ImGui::Text("GetFilePathName() : %s", filePathName.c_str());
                ImGui::Text("GetFilePath() : %s", filePath.c_str());
                ImGui::Text("input directory() : %s", input_directory.c_str());
                ImGui::Text("output directory() : %s", output_directory.c_str());
                ImGui::Text("GetSelection() : ");
                ImGui::Indent();
                {
                    static int selected = false;
                    if (ImGui::BeginTable("##GetSelection", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
                        ImGui::TableSetupScrollFreeze(0, 1);  // Make top row always visible
                        ImGui::TableSetupColumn("File Name", ImGuiTableColumnFlags_WidthStretch, -1, 0);
                        ImGui::TableSetupColumn("File Path name", ImGuiTableColumnFlags_WidthFixed, -1, 1);
                        ImGui::TableHeadersRow();

                        ImGuiListClipper clipper;
                        clipper.Begin((int)selection.size(), ImGui::GetTextLineHeightWithSpacing());
                        while (clipper.Step()) {
                            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                                const auto& sel = selection[i];
                                ImGui::TableNextRow();
                                if (ImGui::TableSetColumnIndex(0))  // first column
                                {
                                    ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
                                    selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
                                    if (ImGui::Selectable(sel.first.c_str(), i == selected, selectableFlags)) selected = i;
                                }
                                if (ImGui::TableSetColumnIndex(1))  // second column
                                {
                                    ImGui::Text("%s", sel.second.c_str());
                                }
                            }
                        }
                        clipper.End();

                        ImGui::EndTable();
                    }
                }
                ImGui::Unindent();
            }

            /////////////////////////////////////////////////////////////////
            //// DISPLAY ////////////////////////////////////////////////////
            /////////////////////////////////////////////////////////////////

            ImVec2 minSize = ImVec2(0, 0);
            ImVec2 maxSize = ImVec2(FLT_MAX, FLT_MAX);

            if (UseWindowContraints) {
                maxSize = ImVec2((float)display_w, (float)display_h) * 0.7f;
                minSize = maxSize * 0.25f;
            }

            // you can define your flags and min/max window size (theses three settings ae defined by default :
            // flags => ImGuiWindowFlags_NoCollapse
            // minSize => 0,0
            // maxSize => FLT_MAX, FLT_MAX (defined is float.h)

            if (fileDialog2.Display("ChooseDirDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
                if (fileDialog2.IsOk()) {
                    filePathName = fileDialog2.GetFilePathName(resultMode);
                    filePath     = fileDialog2.GetCurrentPath();
                    filter       = fileDialog2.GetCurrentFilter();
                    input_directory = filePath;
                    // here convert from string because a string was passed as a userDatas, but it can be what you want
                    if (fileDialog2.GetUserDatas()) userDatas = std::string((const char*)fileDialog2.GetUserDatas());
                    auto sel = fileDialog2.GetSelection(resultMode);  // multiselection
                    selection.clear();
                    for (const auto& s : sel) {
                        selection.emplace_back(s.first, s.second);
                    }
                    // action
                }
                fileDialog2.Close();
            }

            if (fileDialog3.Display("ChooseDirDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
                if (fileDialog3.IsOk()) {
                    filePathName = fileDialog3.GetFilePathName(resultMode);
                    filePath     = fileDialog3.GetCurrentPath();
                    filter       = fileDialog3.GetCurrentFilter();
                    output_directory = filePath;
                    // here convert from string because a string was passed as a userDatas, but it can be what you want
                    if (fileDialog3.GetUserDatas()) userDatas = std::string((const char*)fileDialog3.GetUserDatas());
                    auto sel = fileDialog3.GetSelection(resultMode);  // multiselection
                    selection.clear();
                    for (const auto& s : sel) {
                        selection.emplace_back(s.first, s.second);
                    }
                    // action
                }
                fileDialog3.Close();
            }
        }
        ImGui::End();

        // Cpu Zone : prepare
        ImGui::Render();

        // GPU Zone : Rendering
        glfwMakeContextCurrent(window);

        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#ifdef IMGUI_HAS_VIEWPORT
        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
#endif
        glfwSwapBuffers(window);
    }

#ifdef USE_BOOKMARK
    // remove bookmark
    ImGuiFileDialog::Instance()->RemoveBookmark("Current Dir");

    // save bookmarks dialog 1
    std::ofstream configFileWriter_1("bookmarks_1.conf", std::ios::out);
    if (!configFileWriter_1.bad()) {
        configFileWriter_1 << ImGuiFileDialog::Instance()->SerializeBookmarks();
        configFileWriter_1.close();
    }
    // save bookmarks dialog 2
    std::ofstream configFileWriter_2("bookmarks_2.conf", std::ios::out);
    if (!configFileWriter_2.bad()) {
        configFileWriter_2 << fileDialog2.SerializeBookmarks();
        configFileWriter_2.close();
    }

#endif
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
