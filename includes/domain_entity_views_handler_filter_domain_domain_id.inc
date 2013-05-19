<?php

/**
 * @file
 * Interface between domain_entity.module, domain_views.module and views.module.
 *
 * @ingroup domain_entity
 */

/**
 * Filter for domain ids stored in the database.
 *
 * Access to the domain's list are restricted,
 * depending on the user permissions and assigned domain.
 */
class domain_entity_views_handler_filter_domain_domain_id extends views_handler_filter_in_operator  {

  /**
   * Override views_handler_filter_in_operator::get_value_options().
   * @see views_handler_filter_in_operator::option_definition()
   */
  function option_definition() {
    $options = parent::option_definition();
    $current_domain = domain_get_domain();
    $accessible_domain_ids = domain_entity_get_user_available_domains();
    if (in_array($current_domain['domain_id'], $accessible_domain_ids)) {
      $options['value']['default'] = array('***CURRENT_DOMAIN***');
    }
    else {
      $options['value']['default'] = array(reset($accessible_domain_ids));
    }
    return $options;
  }

  /**
   * Override views_handler_filter_in_operator::get_value_options().
   * @see views_handler_filter_in_operator::get_value_options()
   */
  function get_value_options() {
    if (!isset($this->value_options)) {
      $current_domain = domain_get_domain();
      $accessible_domain_ids = domain_entity_get_user_available_domains();
      $this->value_title = t('Domain');
      $domain_options = array();
      if (in_array($current_domain['domain_id'], $accessible_domain_ids)) {
        $domain_options = array('***CURRENT_DOMAIN***' => t('Current Domain'));
      }
      $domains = domain_domains();
      foreach ($domains as $domain) {
        if (in_array($domain['domain_id'], $accessible_domain_ids)) {
          $domain_options[$domain['domain_id']] = check_plain($domain['sitename']);
        }
      }
      $this->value_options = $domain_options;
    }
  }
}
