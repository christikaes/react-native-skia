#pragma once

#include "JsiSkData.h"
#include <jsi/jsi.h>

namespace RNSkia {

    using namespace facebook;

    class JsiSkDataFactory : public JsiSkHostObject {
    public:
        JSI_HOST_FUNCTION(fromURI) {
            auto jsiLocalUri = arguments[0].asString(runtime);
            auto localUri = jsiLocalUri.utf8(runtime);
            auto context = getContext();
            return react::createPromiseAsJSIValue(
                    runtime,
                    [context, localUri](jsi::Runtime &runtime,
                                        std::shared_ptr<react::Promise> promise) -> void {
                        // Create a stream operation - this will be run in a
                        // separate thread
                        context->performStreamOperation(
                                localUri,
                                [&runtime, context, promise,
                                        localUri](
                                        std::unique_ptr<SkStreamAsset> stream) -> void {
                                    // Schedule drawCallback on the Javascript thread
                                    auto result = SkData::MakeFromStream(stream.get(), stream->getLength());
                                    context->runOnJavascriptThread([&runtime, context, promise,
                                                                           localUri, result]() {
                                        promise->resolve(jsi::Object::createFromHostObject(
                                                runtime,
                                                std::make_shared<JsiSkData>(context,
                                                                                   result)));
                                    });
                        });
            });
        };

        JSI_EXPORT_FUNCTIONS(JSI_EXPORT_FUNC(JsiSkDataFactory, fromURI))

        JsiSkDataFactory(std::shared_ptr<RNSkPlatformContext> context)
                : JsiSkHostObject(context) {}
    };

} // namespace RNSkia
