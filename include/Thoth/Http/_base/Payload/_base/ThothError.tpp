#pragma once

namespace Thoth::Http {
    template<BodyConcept TargetBody>
        requires std::ranges::output_range<TargetBody, char>
    IntoPayload<TargetBody, ThothError>::IntoPayload(ThothError e) : err { std::move(e) } {}

    template<BodyConcept TargetBody>
        requires std::ranges::output_range<TargetBody, char>
    IntoPayload<TargetBody, ThothError>::operator IntoResponsePayload<TargetBody>() && {
        ResponseHead head{
            .status{ std::visit(Hermes::Utils::Overloaded{
                   [](StatusCodeEnum s   ) { return s; },
                   [](ConnectionErrorEnum) { return StatusCodeEnum::InternalServerError; },
                   [](GenericError&      ) { return StatusCodeEnum::InternalServerError; },
                   [](auto&              ) { return StatusCodeEnum::BadRequest; }
               }, err) }
        };

        head.headers.ContentType() = NHeaders::MimeTypes::textPlain;
        TargetBody body;

        if (head.status != StatusCodeEnum::InternalServerError)
            std::format_to(std::back_inserter(body), "{}", err);

        return { std::move(head), std::move(body) };
    }
}