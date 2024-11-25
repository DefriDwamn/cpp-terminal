#include "core/gui_shell.hpp"
#include "core/parser.hpp"
#include "core/virtual_filesystem.hpp"
#include <memory>

GUIShell::GUIShell(std::shared_ptr<VirtualFilesystem> vfs)
    : vfs(vfs), fm(nana::form{}), input_box(fm), output_box(fm) {
  parser = std::make_unique<Parser>(vfs);
  fm.caption("Shell by Yakov");
  fm.size({600, 400});

  output_box.text_align(nana::align::left);
  output_box.editable(false);

  input_box.multi_lines(false);
  input_box.events().key_press([this](const nana::arg_keyboard &arg) {
    if (arg.key == nana::keyboard::enter) {
      on_execute();
    }
  });

  fm.div("vert <output height=90%><input height=10%>");
  fm["output"] << output_box;
  fm["input"] << input_box;
  fm.collocate();
}

void GUIShell::run() {
  fm.show();
  nana::exec();
}

void GUIShell::on_execute() {
  const auto command = input_box.text();
  if (command.empty()) {
    nana::msgbox msg(fm, "Error");
    msg.icon(nana::msgbox::icon_warning) << "Command cannot be empty.";
    msg.show();
    return;
  }
  if (command == "exit") {
    fm.close();
    return;
  }
  if (command == "clear") {
    output_box.caption("");
  }

  try {
    std::string currentDir = vfs->getCurrentDirectory();
    std::string prompt = currentDir + " $ ";

    std::string result = parser->processCommand(command);

    output_box.append("> " + prompt + command + "\n" + result +
                          (result == "" ? "" : "\n"),
                      true);
  } catch (const std::exception &e) {
    nana::msgbox msg(fm, "Error");
    msg.icon(nana::msgbox::icon_error) << e.what();
    msg.show();
  }
  input_box.caption("");
}
