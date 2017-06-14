<?php

class tripal_views_query extends views_plugin_query {

  /**
   * The EntityFieldQuery object used to perform the query
   */
  var $query;

  /**
   * The EntityFieldQuery object used to perform the count query.
   * It will not include the order by and only fields that are used in
   * filters.
   */
  var $cquery;

  /**
   * The fields that are to be included in the query.
   */
  var $fields;

  /**
   * The filters that are to be included in the query.
   */
  var $filters;

  /**
   * The sort item that are to be included in the query.
   */
  var $order;



  /**
   * Ensure a table exists in the queue.
   *
   * This function overrides the views_plugin_query version of the function
   * but does nothing other than return the "table" (or bundle) name as
   * we won't be using aliases for bundles.
   *
   * @param $table
   *   The unaliased name of the table to ensure.
   * @param $relationship
   *   The relationship to ensure the table links to. Each relationship will
   *   get a unique instance of the table being added. If not specified, will
   *   be the primary table.
   * @param $join
   *   A views_join object (or derived object) to join the alias in.
   *
   * @return
   *   The alias used to refer to this specific table, or NULL if the table
   *   cannot be ensured.
   */
  public function ensure_table($table, $relationship = NULL, $join = NULL) {
    // Because we are not querying a table, we're querying a TripalFieldQuery
    // object we don't need to ensure the table.
    return $table;
  }
  /**
   *
   */
  public function init($base_table = 'tripal_entity', $base_field = 'id', $options) {;
    parent::init($base_table, $base_field, $options);
    $this->fields = array();
    $this->where = array();
    $this->order = array();

    // Creqte the TripalFieldQuery object.
    $this->query = new TripalFieldQuery();
    $this->cquery = new TripalFieldQuery();
    $this->cquery->count();

    // Convert the $base_table into the bundle table.  Because every
    // tripal site will have different bundle tables we have to do the
    // conversion for cross-site compatibility.
    list($vocabulary, $accession) = explode('__', $base_table);
    $term = tripal_load_term_entity(array('vocabulary' => $vocabulary, 'accession' => $accession));
    $bundle = tripal_load_bundle_entity(array('term_id' => $term->id));

    // Make sure we only query on the entities for this bundle type.
    $this->query->entityCondition('entity_type', 'TripalEntity');
    $this->query->entityCondition('bundle', $bundle->name);

    $this->cquery->entityCondition('entity_type', 'TripalEntity');
    $this->cquery->entityCondition('bundle', $bundle->name);
  }
  /**
   *
   */
  public function add_field($table_alias, $field_name, $alias = '', $params = array()) {
    $this->fields[] = array(
      'table_alias' => $table_alias,
      'field_name' => $field_name,
      'alias' => $alias,
      'params' => $params
    );
  }

  /**
   * Add a simple WHERE clause to the query.
   *
   * @param $group
   *   The WHERE group to add these to; groups are used to create AND/OR
   *   sections. Groups cannot be nested. Use 0 as the default group. If the
   *   group does not yet exist it will be created as an AND group.
   * @param $field
   *   The name of the field to check.
   * @param $value
   *   The value to test the field against. In most cases, this is a scalar.
   *   For more complex options, it is an array. The meaning of each element
   *   in the array is dependent on the $operator.
   * @param $operator
   *   The comparison operator, such as =, <, or >=. It also accepts more
   *   complex options such as IN, LIKE, or BETWEEN. Defaults to IN if $value
   *   is an array = otherwise. If $field is a string you have to use 'formula'
   *   here.
   */
  public function add_where($group, $field_name, $value = NULL, $operator = NULL) {
    $this->filters[] = array(
      'group' => $group,
      'field_name' => $field_name,
      'value' => $value,
      'op' => $operator
    );

    if ($value) {
      // Handle the bundle properties separate from real fields.
      if ($field_name == 'entity_id' or $field_name == 'status') {
        $this->query->propertyCondition($field_name, $value, $operator);
        $this->cquery->propertyCondition($field_name, $value, $operator);
        return;
      }

      // If the field_name comes to us with a period in it then it means that
      // we need to separate the field name from sub-element names.
      $matches = array();
      if (preg_match('/^(.+?)\.(.*)$/', $field_name, $matches)) {
         $field_name = $matches[1];
         $element_name = $matches[2];
         if (tripal_load_include_field_class($field_name)) {
           $field = field_info_field($field_name);
           $instance = field_info_instance('TripalEntity', $field_name, $this->query->entityConditions['bundle']['value']);
           $field_obj = new $field_name($field, $instance);
           $element_name = $field_obj->getFieldTermID() . ',' . $element_name;
           // Replace periods with commas.
           $element_name = preg_replace('/\./', ',', $element_name);
           $this->query->fieldCondition($field_name, $element_name, $value, $operator);
           $this->cquery->fieldCondition($field_name, $element_name, $value, $operator);
         }
         else {
           throw new Exception("Unkown element id: '$element_name'.");
         }
      }
      else {
        $instance = field_info_instance('TripalEntity', $field_name, $this->query->entityConditions['bundle']['value']);
        $field_term = $instance['settings']['term_vocabulary'] . ':' . $instance['settings']['term_accession'];
        $this->query->fieldCondition($field_name, $field_term, $value, $operator);
        $this->cquery->fieldCondition($field_name, $field_term, $value, $operator);
      }
    }
  }

  /**
   * Overrides add_orderby().
   */
  public function add_orderby($table, $field_name = NULL, $order = 'ASC', $alias = '', $params = array()) {

    if ($field_name) {
      // Make sure we don't put the orderby in more than once.
      foreach ($this->order as $index => $order_details) {
        if ($order_details['field'] == $field_name) {
          return;
        }
      }
      $this->order[] = array(
        'field' => $field_name,
        'direction' => strtoupper($order)
      );

      // If the field_name comes to us with a period in it then it means that
      // we need to separate the field name from sub-element names.
      $matches = array();
      if (preg_match('/^(.+?)\.(.*)$/', $field_name, $matches)) {
        $field_name = $matches[1];
        $element_name = $matches[2];
        $field = field_info_field($field_name);
        $instance = field_info_instance('TripalEntity', $field_name, $this->query->entityConditions['bundle']['value']);
        $element_name = $instance['settings']['term_vocabulary'] . ':' . $instance['settings']['term_accession']  . ',' . $element_name;
        $this->query->fieldOrderBy($field_name, $element_name, $order);

      }
      else {
        $instance = field_info_instance('TripalEntity', $field_name, $this->query->entityConditions['bundle']['value']);
        $field_term = $instance['settings']['term_vocabulary'] . ':' . $instance['settings']['term_accession'];
        $this->query->fieldOrderBy($field_name, $field_term, $order);
      }
    }
  }

  /**
   * Overrides build().
   */
  function build(&$view) {
    // Make the query distinct if the option was set.
    if (!empty($this->options['distinct'])) {
      $this->set_distinct(TRUE, !empty($this->options['pure_distinct']));
    }

    // Store the view in the object to be able to use it later.
    $this->view = $view;

    $view->init_pager();

    // Let the pager modify the query to add limits.
    $this->pager->query();

    $view->build_info['query'] = $this->query;
    $view->build_info['count_query'] = $this->cquery;
  }
  /**
   *
   * @param  $view
   */
  function execute(&$view) {
    $query = $view->build_info['query'];
    $cquery = $view->build_info['count_query'];

    if ($query) {
      $start = microtime(TRUE);

      try {

        if ($this->pager->use_count_query() || !empty($view->get_total_rows)) {
          // TODO: The code below was taken from the
          // views_plugin_pager::execute_count_query($count_query) which would
          // be called here, but that function expects the query is a
          // database query rather than a TripalEntityField query.  We
          // really should create a new tripal_views_plugin_pager class
          // and call the corresponding function here, but due to time
          // constraints this is the shortcut.
          $total_items = $cquery->execute();
          $this->pager->total_items = $total_items;
          if (!empty($this->pager->options['offset'])) {
            $this->pager->total_items -= $this->pager->options['offset'];
          };
          $this->pager->update_page_info();
        }

        // TODO: we need to implement a new views_plugin_pager class to
        // override the pre_execute to set the range, instead we'll just do
        // it manully here until we have the class.
        $this->pager->pre_execute($query);
        $num_items_per_page = $this->pager->get_items_per_page();
        $offset = $this->pager->get_current_page() * $num_items_per_page;
        $query->range($offset, $num_items_per_page);

        // Get the IDs
        $results = $query->execute();
        $entity_ids = array_keys($results['TripalEntity']);

        $this->pager->post_execute($view->result);
        if ($this->pager->use_count_query() || !empty($view->get_total_rows)) {
          $view->total_rows = $this->pager->get_total_items();
        }

        // Get the fields to attach to the entity
        $fields = array();
        $field_ids = array();
        foreach ($this->fields as $details) {
          $field_name = $details['field_name'];
          // If the field_name comes to us with a period in it then it means that
          // we need to separate the field name from sub-element names.
          $matches = array();
          if (preg_match('/^(.+?)\.(.*)$/', $field_name, $matches)) {
            $field_name = $matches[1];
            $element_name = $matches[2];
          }
          $field = field_info_field($field_name);
          if ($field) {
            $fields[$field_name] = $field;
            $field_ids[] = $field['id'];
          }
        }

        // Get the entity IDs from the query.
        $entities = tripal_load_entity('TripalEntity', $entity_ids, FALSE, $field_ids);
        $i = 0;
        foreach ($entities as $entity_id => $entity) {
          $view->result[$i] = new stdClass();
          foreach ($this->fields as $details) {
            $field_name = $details['field_name'];
            // The entity_id and link fields are not true fields. They are
            // added by the tripal_views_data_tripal_entity() function to provide
            // useful fields to reference entities. If we see these
            // we need to support them here by giving them values.
            if ($field_name == 'entity_id') {
              $view->result[$i]->$field_name = $entity;
              continue;
            }
            if ($field_name == 'link') {
              $view->result[$i]->$field_name = $entity;
              continue;
            }
            if ($field_name == 'edit_link') {
              $view->result[$i]->$field_name = $entity;
              continue;
            }
            if ($field_name == 'delete_link') {
              $view->result[$i]->$field_name = $entity;
              continue;
            }
            if ($field_name == 'status') {
              $view->result[$i]->$field_name = $entity->status;
              continue;
            }

            // If the field_name comes to us with a period in it then it means that
            // we need to separate the field name from sub-element names.
            $matches = array();
            if (preg_match('/^(.+?)\.(.*)$/', $field_name, $matches)) {
              $field_name = $matches[1];
              $element_name = $matches[2];
            }

            if (array_key_exists($field_name, $fields)) {
              $items = field_get_items('TripalEntity', $entity, $field_name);
              $view->result[$i]->$field_name = $items;
            }
          }
          // Always add the entity to the results so that handlers
          // can take advantage of it.
          $view->result[$i]->entity = $entity;
          $i++;
        }
      }
      catch (Exception $e) {
        $view->result = array();
        if (!empty($view->live_preview)) {
          drupal_set_message($e->getMessage(), 'error');
        }
        else {
          vpr('Exception in @human_name[@view_name]: @message', array('@human_name' => $view->human_name, '@view_name' => $view->name, '@message' => $e->getMessage()));
        }
      }
    }
    else {
      $start = microtime(TRUE);
    }
    $view->execute_time = microtime(TRUE) - $start;
  }

}