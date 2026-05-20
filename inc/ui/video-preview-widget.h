#ifndef VIDEO_PREVIEW_WIDGET_H_
#define VIDEO_PREVIEW_WIDGET_H_

#include <imgui.h>

class VideoPreviewWidget
{
public:
    VideoPreviewWidget() = default;

    // render_height: 0.0f - take all vertical space
    void render(void* texture_srv, int frame_width, int frame_height, float render_height = 0.0f)
    {
        if (!texture_srv || frame_width == 0 || frame_height == 0)
        {
            ImGui::TextDisabled("Waiting for video stream...");
            return;
        }

        const float aspect_ratio = static_cast<float>(frame_width) / static_cast<float>(frame_height);
        ImVec2 avail_size = ImGui::GetContentRegionAvail();
        
        if (render_height > 0.0f)
            avail_size.y = render_height;

        float draw_width = avail_size.x;
        float draw_height = draw_width / aspect_ratio;

        if (draw_height > avail_size.y)
        {
            draw_height = avail_size.y;
            draw_width = draw_height * aspect_ratio;
        }

        ImVec2 cursor_pos = ImGui::GetCursorPos();
        ImGui::SetCursorPosX(cursor_pos.x + (avail_size.x - draw_width) * 0.5f);
        ImGui::SetCursorPosY(cursor_pos.y + (avail_size.y - draw_height) * 0.5f);

        // D3D11 ImTextureID — pointer to ID3D11ShaderResourceView
        ImGui::Image((ImTextureID)texture_srv, ImVec2(draw_width, draw_height));
    }
};

#endif /* VIDEO_PREVIEW_WIDGET_H_ */