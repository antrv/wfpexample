#include <array>
#include <atomic>
#include <csignal>
#include <memory>
#include <print>
#include <thread>

#include <guiddef.h>
#include <fwpmu.h>

#pragma comment(lib, "Fwpuclnt.lib")

struct EngineDeleter
{
    void operator()(HANDLE handle) const
    {
        FwpmEngineClose(handle);
    }
};

using EnginePtr = std::unique_ptr<std::remove_pointer_t<HANDLE>, EngineDeleter>;

struct BlobDeleter
{
    void operator()(FWP_BYTE_BLOB* blob) const
    {
        FwpmFreeMemory(reinterpret_cast<VOID**>(&blob));
    }
};

using BlobPtr = std::unique_ptr<FWP_BYTE_BLOB, BlobDeleter>;

std::atomic_bool stopRequested {false};

void signalHandler(const int signal) noexcept
{
    if (signal == SIGINT)
    {
        stopRequested = true;
    }
}

int main()
{
    constexpr GUID FIREWALL_BLOCK_UDP_SUBLAYER_KEY =
        {0x3144de8d, 0xe804, 0x4559, {0xb2, 0x10, 0x46, 0x7d, 0x9c, 0xf0, 0x98, 0x40}};

    constexpr GUID FIREWALL_BLOCK_UDP_FILTER_KEY =
        {0x3144de8d, 0xe804, 0x4559, {0xb2, 0x10, 0x46, 0x7d, 0x9c, 0xf0, 0x98, 0x42}};

    std::signal(SIGINT, signalHandler);

    // Create engine
    std::wstring sessionName {L"My Session"};
    std::wstring sessionDesc {L"My Session Description"};

    FWPM_SESSION session {};
    session.displayData.name = sessionName.data();
    session.displayData.description = sessionDesc.data();
    session.flags = FWPM_SESSION_FLAG_DYNAMIC;

    HANDLE tempHandle;
    DWORD errorCode = FwpmEngineOpen(nullptr, RPC_C_AUTHN_WINNT, nullptr, &session, &tempHandle);
    if (errorCode != ERROR_SUCCESS)
    {
        std::println("FwpmEngineOpen failed: {:#x}", errorCode);
        return EXIT_FAILURE;
    }

    EnginePtr engineHandle {tempHandle};

    // Create sub-layer
    std::wstring sublayerName {L"My Sublayer"};
    std::wstring sublayerDesc {L"My Sublayer Description"};

    FWPM_SUBLAYER sublayer {};
    sublayer.subLayerKey = FIREWALL_BLOCK_UDP_SUBLAYER_KEY;
    sublayer.displayData.name = sublayerName.data();
    sublayer.displayData.description = sublayerDesc.data();
    sublayer.flags = 0;
    sublayer.weight = 0x0f;

    errorCode = FwpmSubLayerAdd(engineHandle.get(), &sublayer, nullptr);
    if (errorCode != ERROR_SUCCESS)
    {
        std::println("FwpmSubLayerAdd failed: {:#x}", errorCode);
        return EXIT_FAILURE;
    }

    // Get application path blob
    const std::wstring applicationPath {L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe"};
    FWP_BYTE_BLOB* tempBlob {};
    errorCode = FwpmGetAppIdFromFileName(applicationPath.c_str(), &tempBlob);
    if (errorCode != ERROR_SUCCESS)
    {
        std::println("FwpmGetAppIdFromFileName failed: {:#x}", errorCode);
        return EXIT_FAILURE;
    }

    BlobPtr appIdBlob {tempBlob};

    // Create conditions
    std::array conditions
    {
        FWPM_FILTER_CONDITION {
            .fieldKey = FWPM_CONDITION_IP_PROTOCOL,
            .matchType = FWP_MATCH_EQUAL,
            .conditionValue = {
                .type = FWP_UINT8,
                .uint8 = IPPROTO_UDP
            }
        },
        FWPM_FILTER_CONDITION {
            .fieldKey = FWPM_CONDITION_ALE_APP_ID,
            .matchType = FWP_MATCH_EQUAL,
            .conditionValue = {
                .type = FWP_BYTE_BLOB_TYPE,
                .byteBlob = appIdBlob.get()
            }
        }
    };

    // Create filter
    std::wstring filterName {L"My UDP Block"};
    std::wstring filterDesc {L"My Filter to block UDP for Chrome"};

    FWPM_FILTER filter {};
    filter.filterKey = FIREWALL_BLOCK_UDP_FILTER_KEY;
    filter.layerKey = FWPM_LAYER_ALE_RESOURCE_ASSIGNMENT_V4;
    filter.subLayerKey = sublayer.subLayerKey;
    filter.action.type = FWP_ACTION_BLOCK;
    filter.weight.type = FWP_EMPTY;
    filter.displayData.name = filterName.data();
    filter.displayData.description = filterDesc.data();
    filter.filterCondition = conditions.data();
    filter.numFilterConditions = static_cast<UINT32>(conditions.size());

    UINT64 filterId {};
    errorCode = FwpmFilterAdd(engineHandle.get(), &filter, nullptr, &filterId);
    if (errorCode != ERROR_SUCCESS)
    {
        std::println("FwpmFilterAdd failed: {:#x}", errorCode);
        return EXIT_FAILURE;
    }

    std::println("Press Ctrl+C to exit...");
    while (!stopRequested)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
