#ifndef ALTINTEGRATION_STATE_UTIL_HPP
#define ALTINTEGRATION_STATE_UTIL_HPP

namespace altintegration {

void unapply(PopManager& pop, index_t** popState, index_t& to);

bool apply(PopManager& pop,
           index_t** popState,
           index_t& to,
           ValidationState& state);

bool unapplyAndApply(PopManager& pop,
                     index_t** popState,
                     index_t& to,
                     ValidationState& state);

bool setState(PopManager& pop,
              index_t** popState,
              index_t& to,
              ValidationState& state);

}

#endif  // ALTINTEGRATION_STATE_UTIL_HPP
