#include <string>

namespace tat
{

class VulkansEye
{
  public:
    explicit VulkansEye(const std::string &configPath);
    ~VulkansEye() = default;
    static void run();

  private:

    static void handleInput(float deltaTime);
    static void switchToNormalMode();
    static void switchToVisualMode();
    static void switchToInsertMode();
    
    static void close();
};

} // namespace tat