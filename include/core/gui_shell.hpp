#pragma once
#include "core/parser.hpp"
#include <memory>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>

class GUIShell {
public:
  explicit GUIShell(std::shared_ptr<VirtualFilesystem> vfs);
  void run();

private:
  std::unique_ptr<Parser> parser;
  std::shared_ptr<VirtualFilesystem> vfs;
  
  nana::form fm;
  nana::textbox input_box;
  nana::textbox output_box;

  void on_execute();
};