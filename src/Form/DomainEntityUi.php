<?php

namespace Drupal\domain_entity\Form;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Url;
use Drupal\domain_entity\DomainEntityMapper;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides a form to configure domain field mappings.
 */
class DomainEntityUi extends ConfigFormBase {

  /**
   * The domain entity mapper.
   *
   * @var \Drupal\domain_entity\DomainEntityMapper
   */
  protected $mapper;

  /**
   * Creates a new DomainEntityUi object.
   *
   * @param \Drupal\Core\Config\ConfigFactoryInterface $config_factory
   *   The factory for configuration objects.
   * @param \Drupal\domain_entity\DomainEntityMapper $mapper
   *   The domain entity mapper.
   */
  public function __construct(ConfigFactoryInterface $config_factory, DomainEntityMapper $mapper) {
    parent::__construct($config_factory);
    $this->mapper = $mapper;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('config.factory'),
      $container->get('domain_entity.mapper')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'domain_entity_ui';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $config = $this->config('domain_entity.settings');
    $form['bypass_access_conditions'] = [
      '#title' => $this->t('Disable access rules from this module. (You can use this settings to disable the query alter, for troubleshooting)'),
      '#type' => 'checkbox',
      '#description' => $this->t('When this checkbox is checked, your entities must be accessible on all domains'),
      '#default_value' => $config->get('bypass_access_conditions'),
      '#weight' => -50,
    ];

    $default_values = [];
    foreach ($this->mapper->getEnabledEntityTypes() as $type_id => $entity_type) {
      $default_values[$type_id] = $type_id;
    }

    $form['entity_types'] = [
      '#type' => 'tableselect',
      '#title' => $this->t('Activate domain access on entity types'),
      '#header' => [
        'type' => $this->t('Entity type'),
        'operations' => $this->t('Operations'),
      ],
      '#default_value' => $default_values,
      '#js_select' => FALSE,
    ];
    $rows = [];
    $entity_types = $this->mapper->getEntityTypes();
    foreach ($entity_types as $entity_type_id => $definition) {
      $enabled = !empty($default_values[$entity_type_id]);
      $links = [];
      if ($enabled) {
        $links = [
          'configure' => [
            'title' => $this->t('Configure'),
            'url' => Url::fromRoute('domain_entity.settings', [
              'entity_type_id' => $entity_type_id,
            ]),
          ],
        ];
      }
      $rows[$entity_type_id] = [
        'type' => $definition->getLabel(),
        'operations' => [
          'data' => [
            '#type' => 'operations',
            '#links' => $links,
          ],
        ],
      ];
    }
    $form['entity_types']['#options'] = $rows;

    // @Todo Port active domain UI effects.
    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $config = $this->config('domain_entity.settings');
    $config
      ->set('bypass_access_conditions', $form_state->getValue('bypass_access_conditions'))
      ->save();

    $entity_types = $form_state->getValue('entity_types');
    $all_types = $this->mapper->getEntityTypes();
    $enabled_types = $this->mapper->getEnabledEntityTypes();

    $results = [
      'create' => [],
      'delete' => [],
    ];
    foreach ($all_types as $entity_type_id => $entity_type) {
      if (empty($entity_types[$entity_type_id])) {
        if (isset($enabled_types[$entity_type_id])) {
          $results['delete'][] = $entity_type_id;
        }
      }
      elseif (!isset($enabled_types[$entity_type_id])) {
        $results['create'][] = $entity_type_id;
      }
    }
    // Process results.
    // @todo Add batch to create/delete field storage configs.
    foreach ($results as $action => $types) {
      if ($action == 'delete') {
        foreach ($types as $type) {
          $this->mapper->deleteFieldStorage($type);
        }
      }
      elseif ($action == 'create') {
        foreach ($types as $type) {
          $this->mapper->createFieldStorage($type);
        }
      }
    }

    parent::submitForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return ['domain_entity.settings'];
  }

}
