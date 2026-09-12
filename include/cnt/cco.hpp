#ifndef CNT_CCO_HPP
#define CNT_CCO_HPP

#include <stdexcept>
#include <string>

#include <cnt/cco.h>

/*
 * C++ Exception Exemption Note:
 * According to CPS Section 5.5, C++ exceptions are forbidden in internal
 * engineering code. However, this is a thin public C++ wrapper layer over the C
 * API designed for external consumers. Therefore, exception mapping is exempt
 * and explicitly allowed here.
 */

namespace cnt
{
namespace cco
{

class ParseError : public std::runtime_error
{
  public:
    ParseError(cco_error_t code, const std::string& msg)
        : std::runtime_error(msg), m_code(code)
    {
    }
    cco_error_t code() const { return m_code; }

  private:
    cco_error_t m_code;
};

class Object
{
  public:
    // Takes ownership of a raw C object handle
    explicit Object(cco_object_t* handle) : m_handle(handle) {}

    // Copy construction retains
    Object(const Object& other) : m_handle(other.m_handle)
    {
        if (m_handle)
        {
            cco_object_retain(m_handle);
        }
    }

    // Destructor releases
    ~Object()
    {
        if (m_handle)
        {
            cco_object_release(m_handle);
        }
    }

    // Assignment
    Object& operator=(const Object& other)
    {
        if (this != &other)
        {
            if (m_handle)
            {
                cco_object_release(m_handle);
            }
            m_handle = other.m_handle;
            if (m_handle)
            {
                cco_object_retain(m_handle);
            }
        }
        return *this;
    }

    int type() const { return cco_object_get_type(m_handle); }

    std::string serialize(bool pretty = false) const
    {
        char* cstr = cco_serialize_to_string(m_handle, pretty);
        if (!cstr)
        {
            throw ParseError(CCO_ERR_NO_MEMORY, "Failed to serialize object");
        }
        std::string result(cstr);
        free(
            cstr); // Important: cco_serialize_to_string requires caller to free
        return result;
    }

  private:
    cco_object_t* m_handle;
};

inline Object parse(const std::string& src,
                    const cco_parse_options_t* opts = nullptr)
{
    cco_object_t* obj = cco_parse_string(src.c_str(), src.length(), opts);
    if (!obj)
    {
        cco_error_t err = cco_get_last_error();
        throw ParseError(err, "Failed to parse CCO document");
    }
    return Object(obj);
}

} // namespace cco
} // namespace cnt

#endif // CNT_CCO_HPP
