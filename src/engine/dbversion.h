/*
    SPDX-FileCopyrightText: 2026 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DBVERSION_H
#define BALOO_DBVERSION_H

#include <cstdint>

#include <QDebug>

namespace Baloo
{

class DbVersion
{
public:
    DbVersion() = default;

    /// This is the DB version supported by this
    /// version of the library.
    /// Must be updated whenever the scheme of
    /// the DB changes in an incompatible way.
    static inline const DbVersion& currentDbVersion()
    {
        static DbVersion currentVersion{1, 0};
        return currentVersion;
    }

    /// @return @c true if the version is semantically
    /// valid
    /// Does not reflect if a DB is usable, an outdated
    /// version is still valid.
    bool isValid() const
    {
        return m_major >= 0 && m_minor >= 0;
    }

    bool canRead() const
    {
        return (m_major <= currentDbVersion().m_major);
    }
    bool canWrite() const
    {
        return (m_major == currentDbVersion().m_major);
    }

    int64_t minor() const
    {
        return m_minor;
    }

    int64_t major() const
    {
        return m_major;
    }

    constexpr std::strong_ordering operator<=>(const DbVersion &b) const = default;

    static DbVersion make(std::span<const char, 16> data)
    {
        int64_t ver[2] = {};
        static_assert(sizeof(ver) == data.size());
        memcpy(&ver, data.data(), data.size());
        return DbVersion(ver[0], ver[1]);
    }

    std::array<char, 16> serialize() const
    {
        int64_t ver[2] = {m_major, m_minor};
        std::array<char, 16> data;
        static_assert(sizeof(ver) == data.size());
        memcpy(data.data(), ver, data.size());
        return data;
    }

private:
    DbVersion(int64_t major, int64_t minor)
        : m_major(major)
        , m_minor(minor)
    {}

    int64_t m_major = -1;
    int64_t m_minor = -1;

    friend class DbVersionTest;
};

inline QDebug operator<<(QDebug debug, const DbVersion& v)
{
    if (v.isValid()) {
        QDebugStateSaver saver(debug);
        debug.nospace() << '(' << v.major() << '.' << v.minor() << ')';
    } else {
        debug << "<unknown>";
    }

    return debug;
}

} // namespace Baloo

#endif // BALOO_DBVERSION_H
