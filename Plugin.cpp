
81
82
83
84
85
86
87
88
89
90
91
92
93
94
95
96
97
98
99
100
101
102
103
104
105
106
107
108
109
110
111
112
113
114
115
116
117
118
119
120
121
122
123
124
125
126
127
128
129
130
131
132
133
134
135
136
137
138
139
140
141
142
143
144
145
146
147
148
149
150
151
152
153
154
155
156
157
#include "pch.h"
    void OnF4SEMessage(F4SE::MessagingInterface::Message* a_msg)
    {
        if (!a_msg || !g_runtime) {
            return;
        }

        using MI = F4SE::MessagingInterface;
        switch (a_msg->type) {
        case MI::kGameDataReady:
            // F4SE carries this readiness flag in the message data pointer value
            // (nullptr == false, non-null == true), not as a bool object to dereference.
            if (static_cast<bool>(a_msg->data)) {
                RegisterMenuWatcher();
                QueueApply(true);
            }
            break;

        case MI::kNewGame:
            g_runtime->Invalidate();
            QueueApply(true);
            break;

        case MI::kPreLoadGame:
            g_runtime->Invalidate();
            break;

        case MI::kPostLoadGame:
            // Same F4SE convention as kGameDataReady: pointer truthiness is the
            // success flag. Do not dereference a_msg->data (it may literally be 0x1).
            if (static_cast<bool>(a_msg->data)) {
                RegisterMenuWatcher();
                QueueApply(true);
            }
            break;

        case MI::kGameLoaded:
            RegisterMenuWatcher();
            QueueRefresh(false);
            break;

        case MI::kPreSaveGame:
            g_runtime->SuspendForSave();
            break;

        case MI::kPostSaveGame:
            g_runtime->ResumeAfterSave();
            break;

        default:
            break;
        }
    }
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* a_f4se)
{
    F4SE::Init(a_f4se);

    const auto root = GameRoot();
    const auto configPath = root / "Data" / "F4SE" / "Plugins" / "UniquePlayerRedirector.ini";
    auto config = UPR::Config::Load(configPath);

    REX::INFO("UniquePlayerRedirector 0.2.0 loading");
    REX::INFO("Game root: {}", root.string());
    REX::INFO("Config: {}", configPath.string());

    g_runtime = std::make_unique<UPR::RuntimeForms>(UPR::AssetRouter(std::move(config), root));

    const auto* messaging = F4SE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(OnF4SEMessage)) {
        REX::ERROR("Failed to register F4SE messaging listener");
        return false;
    }

    REX::INFO("UniquePlayerRedirector loaded; ESP/ESL/ESM not required");
    return true;
}
