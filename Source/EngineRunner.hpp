#pragma once

namespace gta2 {

class EngineRunner {
  private:
    bool mIsRunning = false;
    bool mInFrontend = true;
    int mMenuState = 0;

  public:
    bool Initialize(int argc, char** argv);
    bool StepFrame(); // Returns false when the game exits
    void Shutdown();
};

} // namespace gta2