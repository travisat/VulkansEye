#pragma once

#include <memory>

#include <imgui.h>
#include <zep.h>

#include "engine/Window.hpp"

namespace tat
{

static auto zepText = R"R(
// This is a simple demo of using Zep in an ImGui window.  To add this
// to the standard ImGui test app, we just included the zep.h header and
// added simple code to create and display the editor in an ImGui window.
//
// Use the menu in this window to switch between Vim and Notepad style of
// editing.  In Vim Normal mode, try ":e path/to/foo.txt" to open a file,
// or ":tabedit" to add a tab and ":vsplit" to split within a tab.
// 
// Or you can try CTRL+p and start typing a fuzzy searched file name from
// this repository (is instant on release build), followed by ENTER to
// open it, or CTRL+v to open it in a split or CTRL+t to open it in a tab!
//
// It is easy to create new tabs, splits, load files and extend zep.
// Zep even has an abstract file system so that you can provide only the
// files in your game's asset tree, or in a compressed zip file, etc.
void use_zep()
{
    const char* sentiment = "Good luck!";
}
)R";

struct ZepWrapper : Zep::IZepComponent
{
    ZepWrapper() : zepEditor(Zep::ZepPath())
    {
        zepEditor.RegisterCallback(this);
        zepEditor.InitWithText("Test.cpp", zepText);
    }
    Zep::ZepEditor_ImGui zepEditor;

    auto GetEditor() const -> Zep::ZepEditor & override
    {
        return (Zep::ZepEditor &)zepEditor;
    }

    void Notify(std::shared_ptr<Zep::ZepMessage> /*message*/) override
    {
        // Don't care for this demo, but can handle editor events such as buffer changes here
        // or handle special editor commands, etc.
    }
};

class Editor
{
  public:
    void create();
    void show();
    auto getMode() -> Zep::EditorMode
    {
        return zep.zepEditor.GetGlobalMode()->GetEditorMode();
    };

  private:
    ZepWrapper zep;
    Window *window;
};

} // namespace tat