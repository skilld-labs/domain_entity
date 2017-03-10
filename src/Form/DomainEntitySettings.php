<?php

namespace Drupal\domain_entity\Form;

use Drupal\Core\Access\AccessResult;
use Drupal\Core\Entity\EntityTypeBundleInfoInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\domain\DomainLoaderInterface;
use Drupal\domain_entity\DomainEntityMapper;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpKernel\Exception\NotFoundHttpException;

/**
 * Provides a form to configure domain fields mappings.
 */
class DomainEntitySettings extends FormBase {

  /**
   * The entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * The entity type bundle info.
   *
   * @var \Drupal\Core\Entity\EntityTypeBundleInfoInterface
   */
  protected $entityTypeBundleInfo;

  /**
   * The domain entity mapper.
   *
   * @var \Drupal\domain_entity\DomainEntityMapper
   */
  protected $mapper;

  /**
   * The domain loader.
   *
   * @var \Drupal\domain\DomainLoaderInterface
   */
  protected $domainLoader;

  /**
   * The passed entity type.
   *
   * @var \Drupal\Core\Entity\EntityTypeInterface
   */
  protected $entityType;

  /**
   * Creates a new DomainEntityUi object.
   *
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   The entity type manager.
   * @param \Drupal\Core\Entity\EntityTypeBundleInfoInterface $entity_type_bundle_info
   *   The entity type bundle info.
   * @param \Drupal\domain\DomainLoaderInterface $domain_loader
   *   The domain loader.
   * @param \Drupal\domain_entity\DomainEntityMapper $mapper
   *   The domain entity mapper.
   */
  public function __construct(EntityTypeManagerInterface $entity_type_manager, EntityTypeBundleInfoInterface $entity_type_bundle_info, DomainLoaderInterface $domain_loader, DomainEntityMapper $mapper) {
    $this->entityTypeManager = $entity_type_manager;
    $this->entityTypeBundleInfo = $entity_type_bundle_info;
    $this->domainLoader = $domain_loader;
    $this->mapper = $mapper;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('entity_type.manager'),
      $container->get('entity_type.bundle.info'),
      $container->get('domain.loader'),
      $container->get('domain_entity.mapper')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'domain_entity_settings';
  }

  /**
   * The _title_callback for the page of the form.
   *
   * @param string $entity_type_id
   *   The entity type ID.
   *
   * @return \Drupal\Core\StringTranslation\TranslatableMarkup
   *   The page title.
   */
  public function getTitle($entity_type_id = NULL) {
    $entity_type = $this->entityTypeManager->getDefinition($entity_type_id, FALSE);
    return $this->t('Activate domain access on @entity_label (@entity_name)', [
      '@entity_label' => $entity_type->getLabel(),
      '@entity_name' => $entity_type_id,
    ]);
  }

  /**
   * The access check for the form.
   *
   * @param string $entity_type_id
   *   The entity type ID.
   *
   * @return \Drupal\Core\Access\AccessResultInterface
   *   An access result.
   *
   * @throws \Symfony\Component\HttpKernel\Exception\NotFoundHttpException
   */
  public function getFormAccess($entity_type_id) {
    $entity_type = $this->entityTypeManager->getDefinition($entity_type_id, FALSE);
    if (!isset($entity_type)) {
      throw new NotFoundHttpException();
    }
    $enabled_types = $this->mapper->getEnabledEntityTypes();
    if (empty($enabled_types[$entity_type_id])) {
      throw new NotFoundHttpException();
    }
    return AccessResult::allowedIfHasPermission($this->currentUser(), 'administer domains');
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state, $entity_type_id = NULL) {
    $entity_type = $this->entityTypeManager->getDefinition($entity_type_id, FALSE);
    $this->entityType = $entity_type;

    $bundles = $this->getBundles($entity_type_id);
    if (empty($bundles)) {
      $form['info_no_bundles'] = [
        '#markup' => $this->t('Entity @entity_label (@entity_name) has not bundles yet.', [
          '@entity_label' => $entity_type->getLabel(),
          '@entity_name' => $entity_type_id,
        ]),
      ];
      // Early return when no bundles exists.
      return $form;
    }

    // Populate domain list.
    $domain_entity_options = $this->domainLoader->loadOptionsList();
    $domain_entity_behavior = [
      DomainEntityMapper::BEHAVIOR_AUTO => $this->t('Affiliate automatically created entity to a value (no widget on entity creation form, auto-assignation)'),
      DomainEntityMapper::BEHAVIOR_USER => $this->t('User choose affiliate, with a default value (form widget on the entity creation form)'),
    ];

    $has_fields = FALSE;
    foreach ($bundles as $bundle_id => $bundle) {
      $bundle_label = $bundle['label'];
      $settings = [];
      if ($field = $this->mapper->loadField($entity_type_id, $bundle_id)) {
        $settings = $field->getThirdPartySettings('domain_entity');
        $has_fields = TRUE;
      }
      $settings += [
        'domains' => [],
        'behavior' => DomainEntityMapper::BEHAVIOR_AUTO,
      ];
      $form[$bundle_id] = [
        '#type' => 'details',
        '#title' => $bundle_label,
        '#open' => !empty($field),
      ];
      $form[$bundle_id][$bundle_id . '_enable'] = [
        '#title' => $this->t('Enable domain entity access'),
        '#type' => 'checkbox',
        '#default_value' => !empty($field),
      ];
      $states = [
        'visible' => [
          "input[name=\"{$bundle_id}_enable\"]" => ['checked' => TRUE],
        ],
      ];
      $form[$bundle_id][$bundle_id . '_behavior'] = [
        '#title' => $this->t("Choose which behavior must be used with the bundle @bundle_label ('@bundle_name')", [
          '@bundle_label' => $bundle_label,
          '@bundle_name' => $bundle_id,
        ]),
        '#type' => 'select',
        '#options' => $domain_entity_behavior,
        '#default_value' => $settings['behavior'],
        '#states' => $states,
      ];
      $form[$bundle_id][$bundle_id . '_domains'] = [
        '#title' => $this->t("default domain value(s) for the bundle:", [
          '@bundle_label' => $bundle_label,
          '@bundle_name' => $bundle_id,
        ]),
        '#type' => 'checkboxes',
        '#options' => $domain_entity_options,
        '#default_value' => $settings['domains'],
        '#description' => $this->t('When no domains selected entity is available for all domains'),
        '#states' => $states,
      ];
    }

    // Check if content of this type exist in DB, if so prompt a warning.
    $query = $this->entityTypeManager->getStorage($entity_type_id)->getQuery();
    if ($has_fields && $query->accessCheck(FALSE)->range(0, 1)->count()->execute()) {
      $form['message'] = [
        '#theme_wrappers' => [
          'container' => [
            '#attributes' => ['class' => ['messages', 'messages--warning']],
          ],
        ],
        '#markup' => $this->t('* Beware you have entities of this type in your database, all unassigned entities will be assigned to the choosen default domain value(s), if you select "current domain" the unassigned entities will be assigned to the current domain. You can change the default value afterway without altering the existing entities domain value(s).'),
      ];
    }

    $form['actions']['#type'] = 'actions';
    $form['actions']['submit'] = [
      '#type' => 'submit',
      '#value' => $this->t('Save configuration'),
      '#button_type' => 'primary',
    ];
    return $form;
  }

  /**
   * Returns bundles of the entity type.
   *
   * @param string $entity_type_id
   *   The entity type ID.
   *
   * @return array
   *   An array of bundle information where the outer array is keyed by the
   *   bundle name, or the entity type name if the entity does not have bundles.
   *   The inner arrays are associative arrays of bundle information, such as
   *   the label for the bundle.
   */
  protected function getBundles($entity_type_id) {
    return $this->entityTypeBundleInfo->getBundleInfo($entity_type_id);
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $values = $form_state->getValues();
    $entity_type_id = $this->entityType->id();

    $bundles = array_keys($this->getBundles($entity_type_id));
    // @todo Use batch for operations.
    foreach ($bundles as $bundle) {
      if (empty($values[$bundle . '_enable'])) {
        if ($field = $this->mapper->loadField($entity_type_id, $bundle)) {
          $field->delete();
        }
      }
      else {
        if ($field = $this->mapper->loadField($entity_type_id, $bundle)) {
          // Update settings.
          $field->setThirdPartySetting('domain_entity', 'domains', array_filter($values[$bundle . '_domains']));
          $field->setThirdPartySetting('domain_entity', 'behavior', $values[$bundle . '_behavior']);
          $field->save();
        }
        else {
          $this->mapper->addDomainField($entity_type_id, $bundle);
        }
      }
    }
  }

}
