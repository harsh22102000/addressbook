#pragma once

#include <QString>

/**
 * @brief Field-level validation utilities for Contact data.
 *
 * Every method returns an empty string on success, or a human-readable
 * error message that can be displayed directly in the UI.
 */
namespace Validator {

/// Name must be non-empty and at most 100 characters.
QString validateName(const QString &name);

/// Mobile: optional, but if provided must match an acceptable phone pattern.
/// Accepted characters: digits, spaces, '+', '-', '(', ')'.
/// Length: 7–20 significant digits.
QString validateMobile(const QString &mobile);

/// Email: optional, but if provided must be a structurally valid address.
QString validateEmail(const QString &email);

/// Birthday: optional, but if provided must be in the past.
QString validateBirthday(const QString &isoDate);

} // namespace Validator
