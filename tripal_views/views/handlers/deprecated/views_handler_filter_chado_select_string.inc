<?php

/**
 * @file
 * Purpose: This Handler provides a generic select list for any chado field that is a string
 *  The select list includes all distinct values for that field.
 *
 * @ingroup views_filter_handlers
 * @ingroup tripal_core
 */
class views_handler_filter_chado_select_string extends views_handler_filter_string {

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['values_form_type'] = array(
      '#type' => 'radios',
      '#title' => t('Filter Type'),
      '#options' => array(
        'textfield' => 'Text Field',
        'select' => 'Drop-Down Box',
      ),
      '#default_value' => ($this->options['values_form_type']) ? $this->options['values_form_type'] : 'select',
    );

    $form['multiple'] = array(
      '#type' => 'checkbox',
      '#title' => t('Select Multiple'),
      '#description' => t('Allows more then one option to be selected.'),
      '#default_value' => (isset($this->options['multiple'])) ? $this->options['multiple'] : FALSE,
    );

    $form['optional'] = array(
      '#type' => 'checkbox',
      '#title' => t('Optional'),
      '#description' => t('Adds --Any-- to the available options.'),
      '#default_value' => (isset($this->options['optional'])) ? $this->options['optional'] : TRUE,
    );

    $form['max_length'] = array(
      '#type' => 'textfield',
      '#title' => t('Max Width'),
      '#description' => t('Specify the maximum width of the select box'),
      '#default_value' => (isset($this->options['max_length'])) ? $this->options['max_length'] : 40,

    );
    $form['max_length'] = array(
      '#type' => 'markup',
      '#value' => t('<strong><font color="red">Note:</font></strong> If another filter exists for the same table then '.
                    'the values shown in the drop box will only include those from rows that are not filtered.'),

    );

  }

 /**
  * Defines the value field in both the views filter options form
  *   and the exposed form
  */
  function value_form(&$form, &$form_state) {
    parent::value_form($form, $form_state);

    if (preg_match('/textfield/', $this->options['values_form_type'])) {
      $form['value'] = array(
        '#type' => 'textfield',
        '#title' => t('%label', array('%label' => $this->options['label'])),
        '#default_value' => $this->value,
      );

    }
    else {

      // build a where clause that will filter the list in the drop box
      // using fields that are not exposed and that are for the table
      // from whcih the values in the drop box will be slected and
      // we only want to use non-exposed fields because these are not
      // available to the user to edit--their fixed.
      $where = '';
      $filters = $this->view->filter;
      foreach ($filters as $filter_name => $details) {
         // we only want to inclue non-exposed filters 
         if ($details->options['exposed'] == FALSE) {
            // we only want to filter on the table we're getting the list from
            if (strcmp($details->table, $this->table)==0) {
              $where .= "$details->field $details->operator ". $details->value['value'];
              $where .= ' AND ';
            }
         }
      } 
      if ($where) {
         $where = "WHERE $where";
         $where = substr($where, 0, -5); # remove the final ' AND '
      }

      // get the values from the table
      $sql = "SELECT $this->real_field FROM $this->table $where ORDER BY $this->field ASC";
      $results = chado_query($sql);

      // Build the select box options
      $max_length = $this->options['max_length'];
      if (!$max_length) {
        $max_length = 40;
      }
      if ($this->options['optional']) {
        //$options['<select '.$this->table.'>'] = '--None--';
        $options['All'] = '--Any--';
      }
      while ($r = db_fetch_object($results)) {
        if (drupal_strlen($r->{$this->field}) > $max_length) {
          $options[$r->{$this->field}] = drupal_substr($r->{$this->field}, 0, $max_length) . '...';
        }
        else {
          $options[$r->{$this->field}] = $r->{$this->field};
        }
      }

      //Select List
      $form['value'] = array(
          '#type' => 'select',
          '#title' => t('%label', array('%label' => $this->options['label'])),
          '#options' => $options,
          '#default_value' => $this->value,
      );

      if ($this->options['multiple']) {
        $form['value']['#multiple'] = TRUE;
      }
    }
  }

 /**
  * Ensures the select list gets rendered when the filter is exposed
  */
  function exposed_form(&$form, &$form_state) {
    if (empty($this->options['exposed'])) {
      return;
    }

    $value = $this->options['expose']['identifier'];
    $this->value_form($form, $form_state);
    $form[$value] = $form['value'];

    if (isset($form[$value]['#title']) && !empty($form[$value]['#type']) && $form[$value]['#type'] != 'checkbox') {
      unset($form[$value]['#title']);
    }

    $this->exposed_translate($form[$value], 'value');

    if (!empty($form['#type']) && ($form['#type'] == 'checkboxes' || ($form['#type'] == 'select' && !empty($form['#multiple'])))) {
      unset($form[$value]['#default_value']);
    }

    if (!empty($form['#type']) && $form['#type'] == 'select' && empty($form['#multiple'])) {
      $form[$value]['#default_value'] = 'All';
    }

    if ($value != 'value') {
      unset($form['value']);
    }

  }

 /**
  *
  */
  function query() {

    $this->ensure_my_table();
    $field = "$this->table_alias.$this->real_field";
    $upper = $this->case_transform();

    if ($this->options['multiple']) {
      // Remove any if it's there
      unset($this->value['All']);

      if (sizeof($this->value)) {
        $holders = array();
        foreach ($this->value as $v) {
          if (preg_match('/^[\d\.]+$/', $v)) {
            $holders[] = '%d';
          }
          else {
            $holders[] = "'%s'";
          }
        }
        $where = "$field IN (" . implode(", ", $holders) . ")";
        $this->query->add_where($this->options['group'], $where, $this->value);
      }
    }
    else {

      // Deal with All/Any as value
      if (preg_match('/All/', $this->value)) {
        // Don't do anything
      }
      else {
        $info = $this->operators();
        if (!empty($info[$this->operator]['method'])) {
          $this->{$info[$this->operator]['method']}($field, $upper);
        }
      }
    }
  }
}
