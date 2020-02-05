#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_REPOSITORY_EXCEPTIONS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_REPOSITORY_EXCEPTIONS_HPP_

namespace VeriBlock {

namespace db {
/**
 * @class UnrecoverableError
 * @brief Application is not able to recover from the error occurred in the
 * storage.
 *
 * Example: database is corrupted and can not be opened.
 *
 * It is up to implementation to decide a full list of such error types.
 */
struct UnrecoverableError : public std::exception {};

/**
 * @class Error
 * @brief Application in normal condition can recover from this type of error.
 *
 * Example: out of disk space, IO error.
 *
 * It is up to implementation to decide a full list of such error types.
 */
struct Error : public std::exception {};
}  // namespace db

}  // namespace VeriBlock

#endif
