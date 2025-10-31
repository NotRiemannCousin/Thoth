#pragma once
#include <string>
#include <optional>

namespace Thoth::Utils {
    //! @brief Get variables from the ".env" file. Supports command execution and multiline.
    //!
    //! @code
    //! # support comments
    //! UNQUOTED=just the first word
    //! SINGLE='everything between the \'s'
    //! INTERPOLATED="everything between the \'s, but can use thigs like {whoami}"
    //! MULTILINE='''
    //! {can't call commands}
    //! {
    //!     "name": "Connor O'Brian"
    //! }
    //! '''
    //! MULTILINE="""
    //! {echo same as before but can use commands"}
    //! """
    //! @endcode
    std::optional<const std::string_view> Env(std::string_view envkey);

    //! @copybrief Env
    std::optional<const std::u8string_view> Utf8Env(std::string_view envkey);
}
