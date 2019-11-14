#include "Engine.hpp"

namespace tat
{

class VulkansEye
{
  public:
    explicit VulkansEye(const std::string &configPath);
    ~VulkansEye() = default;
    void run();

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
    Engine engine{};

    void handleInput(float deltaTime);
};

} // namespace tat