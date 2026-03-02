#pragma once
namespace Umbra {
    /**
     * Abstract interface for various IMGUI implementations
     **/
    class ImGuiBackend {
    public:
        virtual ~ImGuiBackend() = default;
        /**
         *  Provide initialize imgui context inside provided window
         */
        virtual bool Init(void* window, unsigned int windowWidth, unsigned int windowHeight) = 0;
        /**
         *  Pass along events from window to imgui
         */
        virtual void ProcessEvent(void* event) = 0;
        /**
         *  Clear display and start Drawing
         */
        virtual void NewFrame(float deltaTime) = 0;
        /**
         * Update display to show all the drawn command
         */
        virtual void Render() = 0;
        /**
         * Close and shutdown the imgui context
         */
        virtual void Shutdown() = 0;
    };

} // namespace Umbra
