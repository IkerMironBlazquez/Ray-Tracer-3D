#ifndef RENDER_SCENE_HPP
#define RENDER_SCENE_HPP

#include "material.hpp"
#include "object.hpp"

#include <map>
#include <string>
#include <vector>

namespace render {

  /// Clase para la escena
  class scene {
  public:
    scene() = default;

    /// @throws std::invalid_argument si el material ya existe
    void add_material(material const & mat);

    /// Añade un objeto a la escena
    void add_object(object const & obj);

    [[nodiscard]] bool has_material(std::string const & name) const;

    /// @throws std::invalid_argument si el material no existe
    [[nodiscard]] material const & get_material(std::string const & name) const;

    [[nodiscard]] std::map<std::string, material> const & get_materials() const {
      return materials_;
    }

    [[nodiscard]] std::vector<object> const & get_objects() const { return objects_; }

  private:
    std::map<std::string, material> materials_;
    std::vector<object> objects_;
  };

}  // namespace render

#endif
