<?php

/**
 * @file
 * Domain Access Entity admininistration settings / form
 */

/**
 * Domain Access Entity settings form.
 *
 * @param array $form
 *   Form array.
 * @param array $form_state
 *   Form state array.
 */
function domain_entity_ui($form, &$form_state) {
  // Get the domain_entity_allowed_entity_types values:
  $default_values = domain_entity_allowed_entity_types();
  $entities_info = entity_get_info();
  // Domain access take care of it:
  unset($entities_info['node']);
  // I think this necessit an other module
  // (login, register handling.. to be tested):
  unset($entities_info['user']);
  // Don't know too much about taxonomy vocabulary entity..,
  // for the moment disable it:
  unset($entities_info['taxonomy_vocabulary']);
  // Set the form:
  $form = array();
  $form['header'] = array(
    '#markup' => t('Choose which entities are under Domain Access control, and choose domain entity widget behavior of bundles'),
  );
  $domain_entity_options = array(
    DOMAIN_ENTITY_BEHAVIOR_AUTO => t('Affiliate automatically entities to a value(s)'),
    DOMAIN_ENTITY_BEHAVIOR_USER => t('User choose entities affiliation (with a default value(s))'),
  );
  $form['entities'] = array(
    '#prefix' => '<div id="domain-entity-ui-form-ajax">',
    '#suffix' => '</div>',
  );
  // Construct widget default values options:
  $site_domains = domain_domains();
  $checkboxes = array('current_domain' => t('Current domain'), 'all' => t('Send to all affiliates'));
  foreach ($site_domains as $site_domain) {
    $checkboxes[$site_domain['domain_id']] = $site_domain['sitename'];
  }

  // Now build the form elements.
  foreach ($entities_info as $entity_name => $entity_info) {
    $form['entities'][$entity_name] = array(
      '#title' => '&#x2713; ' . $entity_info['label'] . ' (' . $entity_name . ')',
      '#type' => 'fieldset',
    );
    $form['entities'][$entity_name][$entity_name . '_value'] = array(
      '#title' => t('Activate domain access on @entity_label (@entity_name)', array('@entity_label' => $entity_info['label'], '@entity_name' => $entity_name)),
      '#type' => 'checkbox',
      '#default_value' => isset($default_values[$entity_name]),
      '#ajax' => array(
        'event' => 'change',
        'callback' => 'domain_entity_ui_form_ajax_callback',
        'wrapper' => 'domain-entity-ui-form-ajax',
        'effect' => 'none',
      ),
    );
    foreach ($entity_info['bundles'] as $bundle_name => $bundle_info) {
      $form['entities'][$entity_name]['bundles'][$entity_name . '_' . $bundle_name . '_behavior'] = array(
        '#prefix' => '<fieldset><legend><span class="fieldset-legend">' . $bundle_info['label'] . '</span></legend>',
        '#title' => t("Choose witch behavior Domain entity must use with bundle @bundle_label ('@bundle_name')", array('@bundle_label' => $bundle_info['label'], '@bundle_name' => $bundle_name)),
        '#type' => 'select',
        '#options' => $domain_entity_options,
        '#default_value' => (isset($default_values[$entity_name]) && isset($default_values[$entity_name][$bundle_name])) ? array(key($default_values[$entity_name][$bundle_name]) => key($default_values[$entity_name][$bundle_name])) : array(DOMAIN_ENTITY_BEHAVIOR_USER => DOMAIN_ENTITY_BEHAVIOR_USER),
      );
      // Add a checkboxes for domain id's:
      $form['entities'][$entity_name]['bundles'][$entity_name . '_' . $bundle_name . '_default_values'] = array(
        '#type' => 'checkboxes',
        '#options' => $checkboxes,
        '#title' => t("default domain value(s) for the bundle: @bundle_label ('@bundle_name')", array('@bundle_label' => $bundle_info['label'], '@bundle_name' => $bundle_name)),
        '#default_value' => (isset($default_values[$entity_name]) && isset($default_values[$entity_name][$bundle_name])) ? reset($default_values[$entity_name][$bundle_name]) : array('current_domain' => 'current_domain'),
        '#required' => TRUE,
        '#suffix' => '</fieldset>',
        '#ajax' => array(
          'event' => 'change',
          'callback' => 'domain_entity_ui_form_ajax_callback',
          'wrapper' => 'domain-entity-ui-form-ajax',
        ),
      );

      if (!isset($form_state['values'][$entity_name . '_' . $bundle_name . '_default_values']) && isset($default_values[$entity_name]) && isset($default_values[$entity_name][$bundle_name])) {
        $form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'] = reset($default_values[$entity_name][$bundle_name]);
      }

      if (!empty($form_state['values'])) {
        if (isset($form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'])) {
          if ((isset($form_state['values'][$entity_name . '_' . $bundle_name . '_default_values']['current_domain']) && $form_state['values'][$entity_name . '_' . $bundle_name . '_default_values']['current_domain'] === 'current_domain') || (isset($form_state['values'][$entity_name . '_' . $bundle_name . '_default_values']['all']) && $form_state['values'][$entity_name . '_' . $bundle_name . '_default_values']['all'] === 'all')) {
            foreach ($site_domains as $site_domain) {
              $form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'][$site_domain['domain_id']] = 0;
              $form['entities'][$entity_name]['bundles'][$entity_name . '_' . $bundle_name . '_default_values'][$site_domain['domain_id']]['#disabled'] = TRUE;
            }
            if ((isset($form_state['values'][$entity_name . '_' . $bundle_name . '_default_values']['current_domain']) && $form_state['values'][$entity_name . '_' . $bundle_name . '_default_values']['current_domain'] === 'current_domain')) {
              $form['entities'][$entity_name]['bundles'][$entity_name . '_' . $bundle_name . '_default_values']['all']['#disabled'] = TRUE;
            }
            else {
              $form['entities'][$entity_name]['bundles'][$entity_name . '_' . $bundle_name . '_default_values']['current_domain']['#disabled'] = TRUE;
            }
          }
        }
      }
    }
    if (isset($form_state['triggering_element'])) {
      if (empty($form_state['values']) || !isset($form_state['values'][$entity_name . '_value']) || !$form_state['values'][$entity_name . '_value']) {
        $form['entities'][$entity_name]['bundles']['#access'] = FALSE;
        $form['entities'][$entity_name]['#type'] = 'markup';
        $form['entities'][$entity_name]['#weight'] = 1000;
      }
      else {
        $form['entities'][$entity_name]['bundles']['#access'] = TRUE;
        $form['entities'][$entity_name][$entity_name . '_value']['#title'] = t('un-check for disabling domain access on @entity_label (@entity_name)', array('@entity_label' => $entity_info['label'], '@entity_name' => $entity_name));
        $form['entities'][$entity_name]['#type'] = 'fieldset';
        // Check if content of this type exist in DB, if so prompt a warning:
        if (isset($entity_info['base table'])) {
          $entity_key = $entity_info['entity keys']['id'];
          $query = db_select($entity_info['base table'], 'b');
          $query->addField('b', $entity_key);
          if ($query->execute()->fetchField()) {
            $form['entities'][$entity_name][$entity_name . '_value']['#description']
              = '<div id="edit-changed" class="view-changed messages warning form-wrapper">' .
              t('* Warning you have entities of this type in your database, all unassigned entities will be assigned to the choosen default domain value(s), if you select "current domain" the unassigned entities will be assigned to the current domain. You can change the default value afterway without altering the existing entities domain value(s).') .
              '</div>';
          }
        }
      }
      $form['entities']['#prefix']
        = '<div id="domain-entity-ui-form-ajax">' .
        '<div id="edit-changed" class="view-changed messages warning form-wrapper">' .
        t('* Changes are not saved before you submit this form') .
        '</div>';
    }
    elseif (!isset($default_values[$entity_name])) {
      $form['entities'][$entity_name]['bundles']['#access'] = FALSE;
      $form['entities'][$entity_name]['#type'] = 'markup';
      $form['entities'][$entity_name]['#weight'] = 1000;
    }
    else {
      $form['entities'][$entity_name]['bundles']['#access'] = TRUE;
      $form['entities'][$entity_name][$entity_name . '_value']['#title'] = t('un-check for disabling domain access on @entity_label (@entity_name)', array('@entity_label' => $entity_info['label'], '@entity_name' => $entity_name));
      $form['entities'][$entity_name]['#type'] = 'fieldset';
    }

  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save & Assign existing entities to the default domain'),
  );
  return $form;
}

/**
 * Ajax form callback.
 *
 * @param array $form
 *   Form array.
 *
 * @param array $form_state
 *   Form state array.
 *
 * @return array
 *   Form array of the elements to be updated.
 */
function domain_entity_ui_form_ajax_callback($form, &$form_state) {
  return $form['entities'];

}

/**
 * Submit handler for the domain entity form.
 *
 * @param array $form
 *   Form array.
 *
 * @param array $form_state
 *   Form state array.
 */
function domain_entity_ui_submit($form, &$form_state) {
  $values = $form_state['values'];
  // Set the form.
  $entities_info = entity_get_info();
  // Domain access take care of it:
  unset($entities_info['node']);
  // I think this necessit an other module
  // (login, register handling.. to be tested):
  unset($entities_info['user']);
  // Don't know too much about taxonomy vocabulary entity..,
  // for the moment disable it:
  unset($entities_info['file']);
  unset($entities_info['taxonomy_vocabulary']);

  $domain_entity_allowed_entity_types = array();
  foreach ($entities_info as $entity_name => $entity_info) {
    if ($values[$entity_name . '_value']) {
      $domain_entity_allowed_entity_types[$entity_name] = array();
      foreach ($entity_info['bundles'] as $bundle_name => $bundle_info) {
        $default_values = array();
        foreach ($values[$entity_name . '_' . $bundle_name . '_default_values'] as $key => $value) {
          if ($key == $value && $key !== 'current_domain' && $key !== 'all') {
            $default_values[$key] = $value;
          }
          if ($key === 'current_domain' && $value === 'current_domain') {
            $default_values = array($key => $value);
            break;
          }
          if ($key === 'all' && $value === 'all') {
            $default_values = array($key => $value);
            break;
          }
        }
        $domain_entity_allowed_entity_types[$entity_name][$bundle_name] = array($values[$entity_name . '_' . $bundle_name . '_behavior'] => $default_values);
      }
    }
  }
  $old_domain_entity_allowed_entity_types = variable_get('domain_entity_allowed_entity_types', array());
  if (!empty($old_domain_entity_allowed_entity_types) && !empty($domain_entity_allowed_entity_types)) {
    $uninstall_entity_types = array_diff(array_keys($old_domain_entity_allowed_entity_types), array_keys($domain_entity_allowed_entity_types));
    if (!empty($uninstall_entity_types)) {
      foreach ($uninstall_entity_types as $uninstall_entity_type) {
        $field_name = 'domain_' . $uninstall_entity_type;
        if (strlen($field_name) > 21) {
          $field_name = substr('domain_' . md5($field_name), 0, -7);
        }
        foreach ($old_domain_entity_allowed_entity_types[$uninstall_entity_type] as $bundle_name => $data) {
          $instance = field_info_instance($uninstall_entity_type, $field_name, $bundle_name);
          field_delete_instance($instance, TRUE);
        }
      }
    }
  }
  variable_set('domain_entity_allowed_entity_types', $domain_entity_allowed_entity_types);
  drupal_set_message(t('Domain settings updated'));
  domain_entity_types_enable_domain_field($domain_entity_allowed_entity_types);
}
