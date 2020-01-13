
#pragma once

#ifndef LoraDisplay_h
#define LoraDisplay_h

/////////////////////////////////////////////////////////////////

class LoraDisplay
{
    public:
        LoraDisplay() = default;
        ~LoraDisplay();
        void loop(uint32_t nodeId, int node_list_count, String version, int unread_count);
};

#endif