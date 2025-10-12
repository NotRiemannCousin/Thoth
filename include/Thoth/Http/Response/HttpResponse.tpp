#pragma once


namespace Thoth::Http {
    template<HttpMethodConcept Method>
    HttpResponse<Method>::HttpResponse(HttpVersion version, HttpStatusCodeEnum status, string statusMessage,
            HttpHeaders headers, string body) : version{ version },  status{ status },
            statusMessage{ statusMessage },  headers{ headers }, body{ body } { }
}
