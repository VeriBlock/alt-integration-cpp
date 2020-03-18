#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_CURSOR_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_CURSOR_HPP_

namespace altintegration {

/**
 * Generic map cursor (iterator).
 * @tparam K key type
 * @tparam V value type
 */
template <typename K, typename V>
struct Cursor {
  virtual ~Cursor() = default;

  /**
   * Move cursor to the first key.
   */
  virtual void seekToFirst() = 0;

  /**
   * Move cursor to specific key
   * @param key
   */
  virtual void seek(const K& key) = 0;

  /**
   * Move cursor to the last key
   */
  virtual void seekToLast() = 0;

  /**
   * Does cursor point on a Key-Value pair?
   * @return true if cursor points to element of map, false otherwise
   */
  virtual bool isValid() const = 0;

  /**
   * Move cursor to the next element.
   */
  virtual void next() = 0;

  /**
   * Move cursor to the previous element.
   */
  virtual void prev() = 0;

  /**
   * Get Key in Key-Value mapping pointed by this cursor.
   */
  virtual K key() const = 0;

  /**
   * Get Value in Key-Value mapping pointed by this cursor.
   * @return
   */
  virtual V value() const = 0;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_CURSOR_HPP_
