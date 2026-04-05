from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy


class AuroraEngineConan(ConanFile):
    name = "aurora-engine"
    license = "Proprietary"
    author = "AStruthers2000"
    description = "Aurora Engine - A 2D game engine built with C++23 and SDL3"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    # Export source files needed for building
    exports_sources = "CMakeLists.txt", "src/*", "public/*"

    def requirements(self):
        # GLM math library
        self.requires("glm/1.0.1", transitive_headers=True)
        # SDL3 for graphics and window management
        self.requires("sdl/3.4.0")
        self.requires("sdl_image/3.4.0")

    def configure(self):
        # Force static linking for all dependencies
        self.options["sdl"].shared = False
        self.options["sdl_image"].shared = False
        self.options["catch2"].shared = False

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        # Disable test executable when building for Conan package
        cmake.configure(variables={"BUILD_TESTING": "OFF"})
        cmake.build()

    def package(self):
        # Copy library file to package
        copy(self, "*.lib", src=self.build_folder, dst=f"{self.package_folder}/lib", keep_path=False)
        copy(self, "*.a", src=self.build_folder, dst=f"{self.package_folder}/lib", keep_path=False)

        # Copy public headers to package
        copy(self, "*.h", src=f"{self.source_folder}/public", dst=f"{self.package_folder}/include", keep_path=True)

    def package_info(self):
        # Set library name for linking
        self.cpp_info.libs = ["AuroraEngine"]

        # Set include directories
        self.cpp_info.includedirs = ["include"]

        # Propagate dependencies to consumers
        self.cpp_info.requires = ["glm::glm", "sdl::sdl", "sdl_image::sdl_image"]
        self.cpp_info.set_property("cmake_target_name", "aurora-engine::aurora-engine")
        self.cpp_info.set_property("cmake_file_name", "aurora-engine")
