#pragma once

namespace Thoth::Http {
    template<BodyConcept TargetBody>
        requires std::constructible_from<TargetBody, std::string>
    IntoPayload<TargetBody, Url>::IntoPayload(Url url) : val{ std::move(url) } {}

    template<BodyConcept TargetBody>
        requires std::constructible_from<TargetBody, std::string>
    IntoPayload<TargetBody, Url>::operator IntoRequestPayload<TargetBody>() && {
        auto path{ val.GetPath() };
        std::string baseUrl{ val.GetUrlWithoutFragment().data(),
                                  static_cast<std::size_t>(path.data() + path.size() - val.GetScheme().data()) };
        RequestHead head{};

        head.url = *val.FromUrl(std::move(baseUrl));
        head.headers.ContentType() = NHeaders::MimeTypes::appFormUrlEncoded;
        return { std::move(head), { val.CopyQuery() } };
    }

    template<BodyConcept TargetBody>
        requires std::constructible_from<TargetBody, std::string>
    IntoPayload<TargetBody, Url>::operator IntoResponsePayload<TargetBody>() && {
        ResponseHead head{};
        head.headers.ContentType() = NHeaders::MimeTypes::appFormUrlEncoded;
        return { std::move(head), { val.CopyQuery() } };
    }
}