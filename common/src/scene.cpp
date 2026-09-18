#include "scene.hpp"

#include <stdexcept>

namespace render {

  void scene::add_material(material const & mat) {
    if (has_material(mat.get_name())) {
      throw std::invalid_argument("Material with name [" + mat.get_name() + "] already exists");
    }
    materials_.emplace(mat.get_name(), mat);
  }

  void scene::add_object(object const & obj) {
    objects_.push_back(obj);
  }

  bool scene::has_material(std::string const & name) const {
    return materials_.contains(name);
  }

  material const & scene::get_material(std::string const & name) const {
    auto it = materials_.find(name);
    if (it == materials_.end()) {
      throw std::invalid_argument("Material not found: [" + name + "]");
    }
    return it->second;
  }

}  // namespace render
