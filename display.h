#include <cstdint>
#include <memory>

using std::unique_ptr;

struct IDisplay
{
  virtual void refresh(const uint32_t* pixels) = 0;
};

unique_ptr<IDisplay> createDisplay(int width, int height);

