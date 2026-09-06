#include "pch.h"
#include "AssetRouter.h"
#include "Config.h"
#include "RuntimeForms.h"

namespace
{
    std::unique_ptr<UPR::RuntimeForms> g_runtime;
    bool g_menuSinkRegistered = false;

    std::filesystem::path GameRoot()
    {
        std::array<wchar_t, 32768> buf{};
        const auto len = ::GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
        if (len == 0 || len >= buf.size()) {
            return std::filesystem::current_path();
        }
        return std::filesystem::path(std::wstring_view(buf.data(), len)).parent_path();
    }

    void QueueTask(std::function<void()> a_task)
    {
        if (const auto* tasks = F4SE::GetTaskInterface()) {
            tasks->AddTask(std::move(a_task));
        } else {
            a_task();
        }
    }

    void QueueApply(bool a_reset3D)
    {
        QueueTask([a_reset3D]() {
            if (g_runtime) {
                g_runtime->Apply(a_reset3D);
            }
        });
    }

    void QueueRefresh(bool a_reset3D)
    {
        QueueTask([a_reset3D]() {
            if (g_runtime) {
                g_runtime->Refresh(a_reset3D);
            }
        });
    }

    class MenuWatcher final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent& a_event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
        {
            if (!a_event.opening && g_runtime && g_runtime->WantsMenuRefresh() && !g_runtime->IsSuspended()) {
                QueueRefresh(true);
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };

    MenuWatcher g_menuWatcher;

    void RegisterMenuWatcher()
    {
        if (g_menuSinkRegistered || !g_runtime || !g_runtime->WantsMenuRefresh()) {
            return;
        }
        if (auto* ui = RE::UI::GetSingleton()) {
            ui->RegisterSink<RE::MenuOpenCloseEvent>(&g_menuWatcher);
            g_menuSinkRegistered = true;
            REX::INFO("Menu-close compatibility refresh enabled");
        }
    }

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
