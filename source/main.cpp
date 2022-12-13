// utility functions and includes(logic independent)
#include "utility.hpp"

// std library header
#include <cmath>    // math calculation
#include <fstream>  // file operation
#include <sstream>  // string operation
#include <iostream> // standard input output process
#include <iomanip>  // input output format control
#include <vector>   // container

// Main Window Function
void MainWindowFunction();

// UI layout and UI logic
void ShowMenuBar();
void ShowWindows();
void ShowPorchFileWindow();
void ShowConfirmDeleteWindow();
void ShowToolTip(const char *message);
void UpdateWindows();

// Add new window
void AddDSCWindow(std::string windowName);
void AddNonDSCWindow(std::string windowName);

// window input handler
void DSCInput(int index);
void NonDSCInput(int index);

// window calculate handler
void DSCCalculateAndShow(int index);
void NonDSCCalculateAndShow(int index);

// file operation handler
bool SaveToPorchConfFile(bool isDSC, int index, float input[], float output[]);
bool SaveToPorchConfFile();
bool LoadPorchConfFile();
bool HandlePorchFileHeader(std::string input);
bool DeletePage(int index);

// Porch Conf file path
const std::string PorchConfFilePath = ".\\PorchConf.txt";

// max window number limit
constexpr int MaxDSCWindowNum = 5;
constexpr int MaxNonDSCWindowNum = 5;

// window state
static bool DSCWindowState[MaxDSCWindowNum] = {};
static bool NonDSCWindowState[MaxNonDSCWindowNum] = {};

// window name
static std::string DSCWindowName[MaxDSCWindowNum] = {};
static std::string NonDSCWindowName[MaxNonDSCWindowNum] = {};

// DSC window input buf
static std::string DSCInputTxvid[MaxDSCWindowNum] = {};
static std::string DSCInputHactive[MaxDSCWindowNum] = {};
static std::string DSCInputVactive[MaxDSCWindowNum] = {};
static std::string DSCInputHtotal[MaxDSCWindowNum] = {};
static std::string DSCInputVtotal[MaxDSCWindowNum] = {};
static std::string DSCInputAdjVactive[MaxDSCWindowNum] = {};
static std::string DSCInputAdjHactive[MaxDSCWindowNum] = {};
static std::string DSCInputHFP[MaxDSCWindowNum] = {};
static std::string DSCInputHSYNC[MaxDSCWindowNum] = {};
static std::string DSCInputHBP[MaxDSCWindowNum] = {};

// NonDSC window input buf
static std::string NonDSCInputTxvid[MaxDSCWindowNum] = {};
static std::string NonDSCInputHactive[MaxDSCWindowNum] = {};
static std::string NonDSCInputVactive[MaxDSCWindowNum] = {};
static std::string NonDSCInputHtotal[MaxDSCWindowNum] = {};
static std::string NonDSCInputVtotal[MaxDSCWindowNum] = {};
static std::string NonDSCInputAdjVactive[MaxDSCWindowNum] = {};
static std::string NonDSCInputAdjHactive[MaxDSCWindowNum] = {};
static std::string NonDSCInputHFP[MaxDSCWindowNum] = {};
static std::string NonDSCInputHSYNC[MaxDSCWindowNum] = {};
static std::string NonDSCInputHBP[MaxDSCWindowNum] = {};

// global controller
bool porchFileIsOpen = false;
bool confirmDeletePorchIsOpen = false;
int deletePage = 0;

 // Porch Conf struct
struct Conf
{
public: // Name and type
    std::string porchName = "Default";
    bool isDSC = true;

public: // input data
    float txvid = -1;
    float hactive = -1;
    float vactive = -1;
    float htotal = -1;
    float vtotal = -1;
    float adjVactive = -1;
    float adjHactive = -1;
    float HFP = -1;
    float HSYNC = -1;
    float HBP = -1;

public: // output data
    float fps = -1;
    float adjHtotal = -1;
    float adjHblank = -1;
    float minimumHline = -1;
    float minimumLaneRateSwitchToLPMode = -1;
    float adjHblankMinus40 = -2; // optional
    float hblankMinus40 = -2;
};

// Conf data loaded in memory
static std::vector<Conf> InMemoryData;

int main()
{
    utility::RescaleWindow(); // calculate screen scale factor
    utility::WindowInit();    // init glfw and glad
    utility::GUIInit();       // init imgui
    utility::GUILoadFont();   // init simplified chinese font

    while (!utility::WindowShouldClose())  // close when true
    {
        utility::WindowNewFrame();  // begin glfw frame
        utility::GUINewFrame();     // begin imgui frame

        MainWindowFunction();       // main window function

        utility::GUIDrawFrame();    // end glfw frame
        utility::WindowEndFrame();  // end imgui frame
    }

    utility::GUIDestroy();          // deinit imgui
    utility::WindowDestroy();       // deinit glfw
}

// logic and UI layout entrance
void MainWindowFunction()
{
    ShowMenuBar();    // menu bar layout and logic

    ShowWindows();    // show all windows: DSC/NonDSC/PorchFile/Confirm windows

    UpdateWindows();  // update windows name
}

// menu bar UI layout and logic
void ShowMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Porch Conf File", "Ctrl+O"))
            {
                porchFileIsOpen = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::BeginMenu("Add burst mode DSC Window"))
            {
                static std::string DSCwindowName;
                if (ImGui::Button("Add burst mode DSC window"))
                    AddDSCWindow(DSCwindowName);
                ImGui::SameLine();
                ImGui::InputText("DSC Window Name", &DSCwindowName);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Add burst mode NonDSC Window"))
            {
                static std::string NonDSCwindowName;
                if (ImGui::Button("Add burst mode NonDSC window"))
                    AddNonDSCWindow(NonDSCwindowName);
                ImGui::SameLine();
                ImGui::InputText("NonDSC Window Name", &NonDSCwindowName);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

// show all windows (entrance only)
void ShowWindows()
{
    for (int index = 0; index < MaxDSCWindowNum; index++) // show DSC windows
    {
        if (DSCWindowState[index] == true)
        {
            ImGui::Begin(DSCWindowName[index].c_str(), DSCWindowState + index, ImGuiWindowFlags_AlwaysAutoResize);

            DSCInput(index);
            DSCCalculateAndShow(index);

            ImGui::End();
        }
    }

    for (int index = 0; index < MaxNonDSCWindowNum; index++) // show NonDSC windows
    {
        if (NonDSCWindowState[index] == true)
        {
            ImGui::Begin(NonDSCWindowName[index].c_str(), NonDSCWindowState + index, ImGuiWindowFlags_AlwaysAutoResize);

            NonDSCInput(index);
            NonDSCCalculateAndShow(index);

            ImGui::End();
        }
    }

    if (porchFileIsOpen) // show porch file window
    {
        ShowPorchFileWindow();
    }

    if (confirmDeletePorchIsOpen) // show confirm delete window
    {
        ShowConfirmDeleteWindow();
    }
}

// Porch file window UI layout and logic
void ShowPorchFileWindow()
{
    if (!LoadPorchConfFile())
        return;

    ImGui::SetNextWindowSize(ImVec2(500 * utility::WindowScaleFactor, 440 * utility::WindowScaleFactor));
    if (ImGui::Begin("Porch Conf Window", &porchFileIsOpen, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (InMemoryData.empty())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Conf exists.");
            ImGui::End();
            return;
        }

        // Left
        static int selected = 0;
        {
            ImGui::BeginChild("left pane", ImVec2(150 * utility::WindowScaleFactor, 0), true);
            for (int i = 0; i < InMemoryData.size(); i++)
            {
                std::string label1 = InMemoryData[i].porchName;
                if (ImGui::Selectable(label1.c_str(), selected == i))
                    selected = i;
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        // Right
        {
            ImGui::BeginGroup();
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            ImGui::Text(InMemoryData[selected].porchName.c_str());
            ImGui::SameLine();
            ImGui::Text(InMemoryData[selected].isDSC == true ? "DSC" : "NonDSC");
            ImGui::Separator();

            ImGui::TextColored(ImVec4(0.74117f, 0.98823f, 0.78823f, 1.0f), "txvid:");
            ImGui::Text(std::to_string(InMemoryData[selected].txvid).c_str());
            ImGui::TextColored(ImVec4(0.74117f, 0.98823f, 0.78823f, 1.0f), "hactive:");
            ImGui::Text(std::to_string(InMemoryData[selected].hactive).c_str());
            ImGui::TextColored(ImVec4(0.74117f, 0.98823f, 0.78823f, 1.0f), "vactive:");
            ImGui::Text(std::to_string(InMemoryData[selected].vactive).c_str());
            ImGui::TextColored(ImVec4(0.74117f, 0.98823f, 0.78823f, 1.0f), "htotal:");
            ImGui::Text(std::to_string(InMemoryData[selected].htotal).c_str());
            ImGui::TextColored(ImVec4(0.74117f, 0.98823f, 0.78823f, 1.0f), "vtotal:");
            ImGui::Text(std::to_string(InMemoryData[selected].vtotal).c_str());
            ImGui::TextColored(ImVec4(0.74117f, 0.98823f, 0.78823f, 1.0f), "adj_vactive:");
            ImGui::Text(std::to_string(InMemoryData[selected].adjVactive).c_str());
            ImGui::TextColored(ImVec4(0.74117f, 0.98823f, 0.78823f, 1.0f), "adj_hactive:");
            ImGui::Text(std::to_string(InMemoryData[selected].adjHactive).c_str());
            ImGui::TextColored(ImVec4(0.74117f, 0.98823f, 0.78823f, 1.0f), "HFP:");
            ImGui::Text(std::to_string(InMemoryData[selected].HFP).c_str());
            ImGui::TextColored(ImVec4(0.74117f, 0.98823f, 0.78823f, 1.0f), "HSYNC:");
            ImGui::Text(std::to_string(InMemoryData[selected].HSYNC).c_str());
            ImGui::TextColored(ImVec4(0.74117f, 0.98823f, 0.78823f, 1.0f), "HBP:");
            ImGui::Text(std::to_string(InMemoryData[selected].HBP).c_str());

            ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "帧率:");
            ImGui::Text(std::to_string(InMemoryData[selected].fps).c_str());
            ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "adj_htotal:");
            ImGui::Text(std::to_string(InMemoryData[selected].adjHtotal).c_str());
            ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "adj_hblank:");
            ImGui::Text(std::to_string(InMemoryData[selected].adjHblank).c_str());
            ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "进 LP 时最小 HLINE:");
            ImGui::Text(std::to_string(InMemoryData[selected].minimumHline).c_str());
            ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "切 LP mode, Lane 速率至少需要达到的值:");
            ImGui::Text(std::to_string(InMemoryData[selected].minimumLaneRateSwitchToLPMode).c_str());

            if (InMemoryData[selected].isDSC)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "adj_hblank - 40:");
                ImGui::Text(std::to_string(InMemoryData[selected].adjHblankMinus40).c_str());
                ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "hblank - 40:");
                ImGui::Text(std::to_string(InMemoryData[selected].hblankMinus40).c_str());
            }

            ImGui::EndChild();
            if (ImGui::Button("Delete"))
            {
                deletePage = selected;
                confirmDeletePorchIsOpen = true;
            }
            ImGui::EndGroup();
        }
        ImGui::End();
    }
}

// confirm delete window UI layout and logic
void ShowConfirmDeleteWindow()
{
    ImGui::Begin("Confirm Delete Conf", &confirmDeletePorchIsOpen, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("您是否想删除该配置?");

    if (ImGui::Button("是"))
    {
        DeletePage(deletePage);
        confirmDeletePorchIsOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("否"))
    {
        confirmDeletePorchIsOpen = false;
    }

    ImGui::End();
}

// show tooltip helper
void ShowToolTip(const char *message)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text(message);
        ImGui::EndTooltip();
    }
}

// update DSC/NonDSC window name
void UpdateWindows()
{
    for (int i = 0; i < MaxDSCWindowNum; i++)
    {
        if (DSCWindowState[i] == false)
            DSCWindowName[i] = "NULL";
    }
    for (int i = 0; i < MaxNonDSCWindowNum; i++)
    {
        if (NonDSCWindowState[i] == false)
            NonDSCWindowName[i] = "NULL";
    }
}

// add DSC window
void AddDSCWindow(std::string windowName)
{
    if (windowName.empty() || windowName == "")
        return;

    for (auto &it : DSCWindowName)
        if (it == windowName + " | DSC")
            return;

    int index = 0;
    for (; index < MaxDSCWindowNum; index++)
    {
        if (DSCWindowState[index] == false)
        {
            DSCWindowState[index] = true;
            break;
        }
    }

    if (index >= MaxDSCWindowNum)
        return;

    DSCWindowName[index] = windowName + " | DSC";
}

// add NonDSC window
void AddNonDSCWindow(std::string windowName)
{
    if (windowName.empty() || windowName == "")
        return;

    for (auto &it : NonDSCWindowName)
        if (it == windowName + " | NonDSC")
            return;

    int index = 0;
    for (; index < MaxNonDSCWindowNum; index++)
    {
        if (NonDSCWindowState[index] == false)
        {
            NonDSCWindowState[index] = true;
            break;
        }
    }

    if (index >= MaxNonDSCWindowNum)
        return;

    NonDSCWindowName[index] = windowName + " | NonDSC";
}

// handle DSC window input filed
void DSCInput(int index)
{
    ImGui::InputText("txvid", &DSCInputTxvid[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("video tx 端时钟速率");

    ImGui::InputText("hactive", &DSCInputHactive[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("主机端发出的 H 方向分辨率");

    ImGui::InputText("vactive", &DSCInputVactive[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("主机端发出的 V 方向分辨率");

    ImGui::InputText("htotal", &DSCInputHtotal[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("按此 H 方向配置从 DSC 模块解压图像。\n建议: 将 hsync + hbp 固定为40, 只调hfp。\nHtotal = RX_Hactive + Rx_HSYNC + Rx_HBP + Rx_HFP。\n例如 Htotal = 1080 + 10 + 60 + 30 = 1180");

    ImGui::InputText("vtotal", &DSCInputVtotal[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("按此 V 方向配置从 DSC 模块解压图像。\nVtotal = Rx_Vactive + VSYNC + VFP + VBP\nV 方向的 Porch 值与 TX 屏端的 Porch 值一致。\n例如 Vtotal = 2400 + 4 + 124 + 22 = 2550");

    ImGui::InputText("adj_vactive", &DSCInputAdjVactive[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("该参数为屏端实际要求 V 方向分辨率");

    ImGui::InputText("adj_hactive", &DSCInputAdjHactive[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("该参数为屏端实际要求 H 方向分辨率");

    ImGui::InputText("HFP", &DSCInputHFP[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("寄存器配置中没有这一项按照 HLINE 来求。\nHFP = Hline - TX_Hactive * 3 / 4 - TX_HSYNC - TX_HBP\n例如 HFP = 1042 - 720 * 3 / 4 - 4 - 48 =  450\n在表格中还是填写推荐的 Porch 参与计算, 在代码中填写计算后的数据");

    ImGui::InputText("HSYNC", &DSCInputHSYNC[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("适配屏端参数");

    ImGui::InputText("HBP", &DSCInputHBP[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("适配屏端参数");
}

// handle NonDSC window input filed
void NonDSCInput(int index)
{
    ImGui::InputText("txvid", &NonDSCInputTxvid[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("video tx 端时钟速率");

    ImGui::InputText("hactive", &NonDSCInputHactive[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("缩放后 H 方向分辨率");

    ImGui::InputText("vactive", &NonDSCInputVactive[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("缩放后 V 方向分辨率");

    ImGui::InputText("htotal", &NonDSCInputHtotal[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("video 输出端 H 方向配置\n建议: 将 hsync + hbp 固定为40, 只调hfp。\nHtotal = TX_Hactive + Adj_HSYNC + AdjHBP + AdjHFP\n例如 Htotal = 720 + 16 + 45 + 574 = 1355");

    ImGui::InputText("vtotal", &NonDSCInputVtotal[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("video 输出端 V 方向配置\nVtotal = Tx_active + TX_VSYNC + TX_VBP + TX_VFP 例如 Vtotal = 1600 + 8 + 130 + 110 = 1848");

    ImGui::InputText("adj_vactive", &NonDSCInputAdjVactive[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("该参数为屏端实际要求 V 方向分辨率");

    ImGui::InputText("adj_hactive", &NonDSCInputAdjHactive[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("该参数为屏端实际要求 H 方向分辨率");

    ImGui::InputText("HFP", &NonDSCInputHFP[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("寄存器配置中没有这一项按照 HLINE 来求。\n HFP = Hline - TX_Hactive * 3 / 4 - TX_HSYNC - TX_HBP\n例如 HFP = 1054 - 720 * 3 / 4 - 16 - 45 = 453\n在表格中还是填写推荐的 Porch 参与计算, 在代码中填写计算后的数据");

    ImGui::InputText("HSYNC", &NonDSCInputHSYNC[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("适配屏端参数");

    ImGui::InputText("HBP", &NonDSCInputHBP[index], ImGuiInputTextFlags_CharsDecimal);
    ShowToolTip("适配屏端参数");
}

// handle DSC window value calculation and UI layout and logic
void DSCCalculateAndShow(int index)
{
    float txvid, hactive, vactive, htotal, vtotal, adjVactive, adjHactive, HFP, HSYNC, HBP;

    if (!DSCInputTxvid[index].empty())
        txvid = std::stof(DSCInputTxvid[index]);
    if (!DSCInputHactive[index].empty())
        hactive = std::stof(DSCInputHactive[index]);
    if (!DSCInputVactive[index].empty())
        vactive = std::stof(DSCInputVactive[index]);
    if (!DSCInputHtotal[index].empty())
        htotal = std::stof(DSCInputHtotal[index]);
    if (!DSCInputVtotal[index].empty())
        vtotal = std::stof(DSCInputVtotal[index]);
    if (!DSCInputAdjVactive[index].empty())
        adjVactive = std::stof(DSCInputAdjVactive[index]);
    if (!DSCInputAdjHactive[index].empty())
        adjHactive = std::stof(DSCInputAdjHactive[index]);
    if (!DSCInputHFP[index].empty())
        HFP = std::stof(DSCInputHFP[index]);
    if (!DSCInputHSYNC[index].empty())
        HSYNC = std::stof(DSCInputHSYNC[index]);
    if (!DSCInputHBP[index].empty())
        HBP = std::stof(DSCInputHBP[index]);

    float fps, adjHtotal, adjHblank, minimumHline, minimumLaneRateSwitchToLPMode, adjHblankMinus40, hblankMinus40;

    // frame rate
    if (!(DSCInputTxvid[index].empty() || DSCInputVactive[index].empty() || DSCInputHtotal[index].empty() || DSCInputVtotal[index].empty() || DSCInputAdjVactive[index].empty()))
    {
        adjHtotal = (vactive) * (htotal) / (adjVactive);
        fps = (txvid * 1000000.0f) / (adjHtotal * (((vtotal) - (vactive)) + (adjVactive)));
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "帧率:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", fps);
        ShowToolTip("输出帧率由 video TX 端参数与 txvid 共同确定");
    }

    // adjHtotal
    if (!(DSCInputVactive[index].empty() || DSCInputHtotal[index].empty() || DSCInputAdjVactive[index].empty()))
    {
        adjHtotal = (vactive) * (htotal) / (adjVactive);
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "adj_htotal:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", adjHtotal);
        ShowToolTip("该参数为 video 输出端缩放适配参数,\n通过公式: vactive * htotal = adj_vactive * adj_htotal 计算得到");
    }

    // adjHblank
    if (!(DSCInputVactive[index].empty() || DSCInputHtotal[index].empty() || DSCInputAdjVactive[index].empty() || DSCInputHactive[index].empty()))
    {
        adjHtotal = (vactive) * (htotal) / (adjVactive);
        adjHblank = adjHtotal - adjHactive;
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "adj_hblank:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", adjHblank);
        ShowToolTip("adj_hblank = adj_hfp + adj_hsync + adj_hbp + adj_active\n建议: 将 adj_hsync + adj_hbp 固定为40, 只调 adj_hfp");
    }

    // minimumHline
    if (!(DSCInputAdjHactive[index].empty() || DSCInputHSYNC[index].empty() || DSCInputHBP[index].empty()))
    {
        minimumHline = std::ceil((static_cast<float>(4 * 50 + 4 * 96 + 4 * 40 + 14 + 3 * (adjHactive) + 4 * (HSYNC) + 4 * (HBP))) / (3.0f));
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "进LP时最小HLINE:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", minimumHline);
        ShowToolTip("对于burst模式, 协议要求行将切到LP模式。\n若要切LP模式, HLINE_TIME配置的最小值, 需要满足公式:\nbllphytesremain > mintime2lp_plus4");
    }

    // minimumLaneRateToSwitchToLPMode
    if (!(DSCInputTxvid[index].empty() || DSCInputVactive[index].empty() || DSCInputHtotal[index].empty() || DSCInputAdjVactive[index].empty() || DSCInputAdjHactive[index].empty() || DSCInputHFP[index].empty() || DSCInputHSYNC[index].empty() || DSCInputHBP[index].empty()))
    {
        adjHtotal = (vactive) * (htotal) / (adjVactive);
        minimumLaneRateSwitchToLPMode = utility::ceiling((((adjHactive)*3.0f / 4.0f + (HFP) + (HSYNC) + (HBP)) * ((txvid) / adjHtotal)) * 8.0f * 1.2f, 25);
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "切LP mode, Lane速率至少需要达到的值:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", minimumLaneRateSwitchToLPMode);
        ShowToolTip("对于 burst 模式, 协议要求行将切到 LP 模式。\n若要切 LP mode, Lane 速率至少需要达到的值");
    }

    // adjHblank - 40
    if (!(DSCInputVactive[index].empty() || DSCInputHtotal[index].empty() || DSCInputAdjVactive[index].empty() || DSCInputAdjHactive[index].empty()))
    {
        adjHtotal = (vactive) * (htotal) / (adjVactive);
        adjHblank = adjHtotal - adjHactive;
        adjHblankMinus40 = adjHblank - 40;
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "adj_hblank - 40:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", adjHblankMinus40);
    }

    // hblank - 40
    if (!(DSCInputHactive[index].empty() || DSCInputHtotal[index].empty()))
    {
        hblankMinus40 = (htotal)-40 - (hactive);
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "hblank - 40:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", hblankMinus40);
    }

    static bool isSave = false;
    float input[10] = {txvid, hactive, vactive, htotal, vtotal, adjVactive, adjHactive, HFP, HSYNC, HBP};
    float output[7] = {fps, adjHtotal, adjHblank, minimumHline, minimumLaneRateSwitchToLPMode, adjHblankMinus40, hblankMinus40};
    if (ImGui::Button("Save To File"))
        isSave = SaveToPorchConfFile(true, index, input, output);
    ImGui::SameLine();
    if (isSave)
        ImGui::Text("Saved successfully.");
    else
        ImGui::Text("Unsaved.");
    ShowToolTip("You must type all data to save it into the PorchConf.txt file.");
}

// handle NonDSC window value calculation and UI layout and logic
void NonDSCCalculateAndShow(int index)
{
    float txvid, hactive, vactive, htotal, vtotal, adjVactive, adjHactive, HFP, HSYNC, HBP;

    if (!NonDSCInputTxvid[index].empty())
        txvid = std::stof(NonDSCInputTxvid[index]);
    if (!NonDSCInputHactive[index].empty())
        hactive = std::stof(NonDSCInputHactive[index]);
    if (!NonDSCInputVactive[index].empty())
        vactive = std::stof(NonDSCInputVactive[index]);
    if (!NonDSCInputHtotal[index].empty())
        htotal = std::stof(NonDSCInputHtotal[index]);
    if (!NonDSCInputVtotal[index].empty())
        vtotal = std::stof(NonDSCInputVtotal[index]);
    if (!NonDSCInputAdjVactive[index].empty())
        adjVactive = std::stof(NonDSCInputAdjVactive[index]);
    if (!NonDSCInputAdjHactive[index].empty())
        adjHactive = std::stof(NonDSCInputAdjHactive[index]);
    if (!NonDSCInputHFP[index].empty())
        HFP = std::stof(NonDSCInputHFP[index]);
    if (!NonDSCInputHSYNC[index].empty())
        HSYNC = std::stof(NonDSCInputHSYNC[index]);
    if (!NonDSCInputHBP[index].empty())
        HBP = std::stof(NonDSCInputHBP[index]);

    float fps, adjHtotal, adjHblank, minimumHline, minimumLaneRateSwitchToLPMode;

    // frame rate
    if (!(NonDSCInputTxvid[index].empty() || NonDSCInputHtotal[index].empty() || NonDSCInputVtotal[index].empty()))
    {
        fps = (txvid * 1000000.0f) / (htotal * vtotal);
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "帧率:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", fps);
        ShowToolTip("输出帧率由 video TX 端参数与 txvid 共同确定");
    }

    // adjHtotal
    if (!(NonDSCInputVactive[index].empty() || NonDSCInputHtotal[index].empty() || NonDSCInputAdjVactive[index].empty()))
    {
        adjHtotal = (vactive) * (htotal) / (adjVactive);
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "adj_htotal:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", adjHtotal);
        ShowToolTip("该参数与 htotal 一致");
    }

    // adjHblank
    if (!(NonDSCInputVactive[index].empty() || NonDSCInputHtotal[index].empty() || NonDSCInputAdjVactive[index].empty() || NonDSCInputHactive[index].empty()))
    {
        adjHtotal = (vactive) * (htotal) / (adjVactive);
        adjHblank = adjHtotal - adjHactive;
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "adj_hblank:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", adjHblank);
        ShowToolTip("adj_hblank = adj_hfp + adj_hsync + adj_hbp + adj_active\n建议: 将 adj_hsync + adj_hbp 固定为40, 只调 adj_hfp");
    }

    // minimumHline
    if (!(NonDSCInputAdjHactive[index].empty() || NonDSCInputHSYNC[index].empty() || NonDSCInputHBP[index].empty()))
    {
        minimumHline = std::ceil((static_cast<float>(4 * 50 + 4 * 96 + 4 * 40 + 14 + 3 * (adjHactive) + 4 * (HSYNC) + 4 * (HBP))) / (3.0f));
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "进LP时最小HLINE:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", minimumHline);
        ShowToolTip("对于burst模式, 协议要求行将切到LP模式。\n若要切LP模式, HLINE_TIME配置的最小值, 需要满足公式:\nbllphytesremain > mintime2lp_plus4");
    }

    // minimumLaneRateToSwitchToLPMode
    if (!(NonDSCInputTxvid[index].empty() || NonDSCInputVactive[index].empty() || NonDSCInputHtotal[index].empty() || NonDSCInputAdjVactive[index].empty() || NonDSCInputAdjHactive[index].empty() || NonDSCInputHFP[index].empty() || NonDSCInputHSYNC[index].empty() || NonDSCInputHBP[index].empty()))
    {
        adjHtotal = (vactive) * (htotal) / (adjVactive);
        minimumLaneRateSwitchToLPMode = utility::ceiling((((adjHactive)*3.0f / 4.0f + (HFP) + (HSYNC) + (HBP)) * ((txvid) / adjHtotal)) * 8.0f * 1.2f, 25);
        ImGui::TextColored(ImVec4(1.0f, 0.92156f, 0.80392f, 1.0f), "切LP mode, Lane速率至少需要达到的值:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.4f", minimumLaneRateSwitchToLPMode);
        ShowToolTip("对于 burst 模式, 协议要求行将切到 LP 模式。\n若要切 LP mode, Lane 速率至少需要达到的值");
    }

    static bool isSave = false;
    float input[10] = {txvid, hactive, vactive, htotal, vtotal, adjVactive, adjHactive, HFP, HSYNC, HBP};
    float output[5] = {fps, adjHtotal, adjHblank, minimumHline, minimumLaneRateSwitchToLPMode};
    if (ImGui::Button("Save To File"))
        isSave = SaveToPorchConfFile(false, index, input, output);
    ImGui::SameLine();
    if (isSave)
        ImGui::Text("Saved successfully.");
    else
        ImGui::Text("Unsaved.");
    ShowToolTip("You must type all data to save it into the PorchConf.txt file.");
}

// save to porch file from DSC/NonDSC window
bool SaveToPorchConfFile(bool isDSC, int index, float input[], float output[])
{
    if (isDSC)
    {
        if (DSCInputTxvid[index].empty() ||
            DSCInputHactive[index].empty() ||
            DSCInputVactive[index].empty() ||
            DSCInputHtotal[index].empty() ||
            DSCInputVtotal[index].empty() ||
            DSCInputAdjVactive[index].empty() ||
            DSCInputAdjHactive[index].empty() ||
            DSCInputHFP[index].empty() ||
            DSCInputHSYNC[index].empty() ||
            DSCInputHBP[index].empty())
            return false;
    }
    else
    {
        if (NonDSCInputTxvid[index].empty() ||
            NonDSCInputHactive[index].empty() ||
            NonDSCInputVactive[index].empty() ||
            NonDSCInputHtotal[index].empty() ||
            NonDSCInputVtotal[index].empty() ||
            NonDSCInputAdjVactive[index].empty() ||
            NonDSCInputAdjHactive[index].empty() ||
            NonDSCInputHFP[index].empty() ||
            NonDSCInputHSYNC[index].empty() ||
            NonDSCInputHBP[index].empty())
            return false;
    }

    std::ofstream outStream;
    outStream.open(PorchConfFilePath, std::ofstream::out | std::ios::app); // append to the end of the file
    if (!outStream)
    {
        std::cerr << "Error: Unable to open Porch Conf file!" << std::endl;
        return false;
    }

    // header
    outStream << "0.  "
              << "============================================================" << std::endl
              << std::left;

    outStream << "1.  " << std::setw(25) << (isDSC ? DSCWindowName[index] : NonDSCWindowName[index]) << "Porch Name" << std::endl;
    outStream << "2.  " << std::setw(25) << (isDSC ? "DSC" : "NonDSC") << "Porch Type" << std::endl;
    outStream << "3.  "
              << "---------------------------input----------------------------" << std::endl;
    outStream << "4.  " << std::setw(15) << input[0] << "txvid" << std::endl;
    outStream << "5.  " << std::setw(15) << input[1] << "hactive" << std::endl;
    outStream << "6.  " << std::setw(15) << input[2] << "vactive" << std::endl;
    outStream << "7.  " << std::setw(15) << input[3] << "htotal" << std::endl;
    outStream << "8.  " << std::setw(15) << input[4] << "vtotal" << std::endl;
    outStream << "9.  " << std::setw(15) << input[5] << "adj_vactive" << std::endl;
    outStream << "10. " << std::setw(15) << input[6] << "adj_hactive" << std::endl;
    outStream << "11. " << std::setw(15) << input[7] << "HFP" << std::endl;
    outStream << "12. " << std::setw(15) << input[8] << "HSYNC" << std::endl;
    outStream << "13. " << std::setw(15) << input[9] << "HBP" << std::endl;
    outStream << "14. "
              << "---------------------------output---------------------------" << std::endl;
    outStream << "15. " << std::setw(15) << output[0] << "帧率" << std::endl;
    outStream << "16. " << std::setw(15) << output[1] << "adj_htotal" << std::endl;
    outStream << "17. " << std::setw(15) << output[2] << "adj_hblank" << std::endl;
    outStream << "18. " << std::setw(15) << output[3] << "进 LP 时最小 HLINE" << std::endl;
    outStream << "19. " << std::setw(15) << output[4] << "切 LP mode, Lane 速率至少需要达到的值" << std::endl;

    if (isDSC)
    {
        outStream << "20. " << std::setw(15) << output[5] << "adj_hblank - 40" << std::endl;
        outStream << "21. " << std::setw(15) << output[6] << "hblank - 40" << std::endl;
    }

    outStream.close();
    return true;
}

// save to porch file from delete event
bool SaveToPorchConfFile()
{
    std::ofstream outStream;
    outStream.open(PorchConfFilePath, std::ofstream::out); // overwrite the file
    if (!outStream)
    {
        std::cerr << "Error: Unable to open Porch Conf file!" << std::endl;
        return false;
    }

    outStream << "PorchConfHeader@zionFisher //Do not delete this line or you will not be able to open this file" << std::endl;

    for (auto &it : InMemoryData)
    {
        // header
        outStream << "0.  "
                  << "============================================================" << std::endl
                  << std::left;

        outStream << "1.  " << std::setw(25) << (it.isDSC ? (it.porchName + " | DSC") : (it.porchName + " | NonDSC")) << "Porch Name" << std::endl;
        outStream << "2.  " << std::setw(25) << (it.isDSC ? "DSC" : "NonDSC") << "Porch Type" << std::endl;
        outStream << "3.  "
                  << "---------------------------input----------------------------" << std::endl;
        outStream << "4.  " << std::setw(15) << it.txvid << "txvid" << std::endl;
        outStream << "5.  " << std::setw(15) << it.hactive << "hactive" << std::endl;
        outStream << "6.  " << std::setw(15) << it.vactive << "vactive" << std::endl;
        outStream << "7.  " << std::setw(15) << it.htotal << "htotal" << std::endl;
        outStream << "8.  " << std::setw(15) << it.vtotal << "vtotal" << std::endl;
        outStream << "9.  " << std::setw(15) << it.adjVactive << "adj_vactive" << std::endl;
        outStream << "10. " << std::setw(15) << it.adjHactive << "adj_hactive" << std::endl;
        outStream << "11. " << std::setw(15) << it.HFP << "HFP" << std::endl;
        outStream << "12. " << std::setw(15) << it.HSYNC << "HSYNC" << std::endl;
        outStream << "13. " << std::setw(15) << it.HBP << "HBP" << std::endl;
        outStream << "14. "
                  << "---------------------------output---------------------------" << std::endl;
        outStream << "15. " << std::setw(15) << it.fps << "帧率" << std::endl;
        outStream << "16. " << std::setw(15) << it.adjHtotal << "adj_htotal" << std::endl;
        outStream << "17. " << std::setw(15) << it.adjHblank << "adj_hblank" << std::endl;
        outStream << "18. " << std::setw(15) << it.minimumHline << "进 LP 时最小 HLINE" << std::endl;
        outStream << "19. " << std::setw(15) << it.minimumLaneRateSwitchToLPMode << "切 LP mode, Lane 速率至少需要达到的值" << std::endl;

        if (it.isDSC)
        {
            outStream << "20. " << std::setw(15) << it.adjHblankMinus40 << "adj_hblank - 40" << std::endl;
            outStream << "21. " << std::setw(15) << it.hblankMinus40 << "hblank - 40" << std::endl;
        }
    }

    outStream.close();
    LoadPorchConfFile();
    return true;
}

// load porch file
bool LoadPorchConfFile()
{
    InMemoryData.clear(); // clear legacy data at very first

    std::ifstream inStream;
    inStream.open(PorchConfFilePath, std::ofstream::in);
    if (!inStream)
    {
        std::cerr << "Error: Unable to open Porch Conf file!" << std::endl;
        return false;
    }

    std::string lineBuf;
    std::getline(inStream, lineBuf);
    if (lineBuf != "PorchConfHeader@zionFisher //Do not delete this line or you will not be able to open this file")
    {
        std::cerr << "Error: The header is wrong, please change the first line of the file to \"PorchConfHeader@zionFisher //Do not delete this line or you will not be able to open this file]\"" << std::endl;
        inStream.close();
        return false;
    }

    while (std::getline(inStream, lineBuf))
    {
        if (!HandlePorchFileHeader(lineBuf))
        {
            inStream.close();
            return false;
        }
    }

    inStream.close();
    return true;
}

// handle each line of the file
bool HandlePorchFileHeader(std::string input)
{
    std::istringstream strStream(input);
    std::string prefix;
    strStream >> prefix;

    if (prefix == "0.")
    {
        InMemoryData.emplace_back(Conf());
        return true;
    }
    if (prefix == "3." || prefix == "14.")
    {
        return true;
    }
    if (prefix == "1.")
    {
        int separator = input.find_last_of('|');
        std::string cur;
        while (strStream >> prefix)
        {
            if (prefix == "|")
                break;

            cur += prefix;
        };
        InMemoryData.back().porchName = cur;
        return true;
    }
    if (prefix == "2.")
    {
        strStream >> prefix;
        InMemoryData.back().isDSC = prefix == "DSC" ? true : false;
        return true;
    }
    float buf;
    strStream >> buf;
    if (prefix == "4.")
    {
        InMemoryData.back().txvid = buf;
        return true;
    }
    if (prefix == "5.")
    {
        InMemoryData.back().hactive = buf;
        return true;
    }
    if (prefix == "6.")
    {
        InMemoryData.back().vactive = buf;
        return true;
    }
    if (prefix == "7.")
    {
        InMemoryData.back().htotal = buf;
        return true;
    }
    if (prefix == "8.")
    {
        InMemoryData.back().vtotal = buf;
        return true;
    }
    if (prefix == "9.")
    {
        InMemoryData.back().adjVactive = buf;
        return true;
    }
    if (prefix == "10.")
    {
        InMemoryData.back().adjHactive = buf;
        return true;
    }
    if (prefix == "11.")
    {
        InMemoryData.back().HFP = buf;
        return true;
    }
    if (prefix == "12.")
    {
        InMemoryData.back().HSYNC = buf;
        return true;
    }
    if (prefix == "13.")
    {
        InMemoryData.back().HBP = buf;
        return true;
    }
    if (prefix == "15.")
    {
        InMemoryData.back().fps = buf;
        return true;
    }
    if (prefix == "16.")
    {
        InMemoryData.back().adjHtotal = buf;
        return true;
    }
    if (prefix == "17.")
    {
        InMemoryData.back().adjHblank = buf;
        return true;
    }
    if (prefix == "18.")
    {
        InMemoryData.back().minimumHline = buf;
        return true;
    }
    if (prefix == "19.")
    {
        InMemoryData.back().minimumLaneRateSwitchToLPMode = buf;
        return true;
    }
    if (prefix == "20.")
    {
        InMemoryData.back().adjHblankMinus40 = buf;
        return true;
    }
    if (prefix == "21.")
    {
        InMemoryData.back().hblankMinus40 = buf;
        return true;
    }

    std::cerr << "Error: incorrect porch file format" << std::endl;
    return false;
}

// delete specified entry in InMemoryData
bool DeletePage(int index)
{
    InMemoryData.erase(InMemoryData.begin() + index);

    return SaveToPorchConfFile();
}
