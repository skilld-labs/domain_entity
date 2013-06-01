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
   * @see views_handler_filter_in_operator::get_value_options()
   */
  function get_value_options() {
    if (!isset($this->value_options)) {
      $current_domain = domain_get_domain();
      $accessible_domain_ids = domain_entity_get_user_available_domains();
      $domains = domain_domains();
      $domain_options = array();
      $this->value_title = t('Domain');
      if (count($accessible_domain_ids) == count($domains)) {
        $domain_options[DOMAIN_ALL] = t('All');
      }
      if (in_array($current_domain['domain_id'], $accessible_domain_ids)) {
        $domain_options['***CURRENT_DOMAIN***'] = t('Current Domain');
      }
      foreach ($domains as $domain) {
        if (in_array($domain['domain_id'], $accessible_domain_ids)) {
          $domain_options[$domain['domain_id']] = check_plain($domain['sitename']);
        }
      }

      $this->value_options = $domain_options;
    }
  }

  /**
   * Override views_handler_filter_in_operator::value_form().
   * @see views_handler_filter_in_operator::value_form()
   */
  function value_form(&$form, &$form_state) {
    parent::value_form($form, $form_state);
    if (isset($form_state['input'][$this->real_field]) && $form_state['input'][$this->real_field] != NULL) {
      $current_domain = domain_get_domain();
      $accessible_domain_ids = domain_entity_get_user_available_domains();
      $domains = domain_domains();

      $possible_options = array_keys($this->value_options);

      // Check if the input is allowed.
      $value = $form_state['input'][$this->real_field];
      if (is_array($value)) {
        $value = reset($value);
      }
      if (!in_array($value, $possible_options)) {
        $form_state['input'][$this->real_field] = NULL;
      }
    }

  }
  /**
   * Override views_handler_filter_in_operator::query().
   *
   * Do not add the condition to the query
   * if the value is a joker.
   *
   * @see views_handler_filter_in_operator::query()
   */
  function query() {
    if (in_array(DOMAIN_ALL, $this->value)) {
      return;
    }
    parent::query();
  }
}
